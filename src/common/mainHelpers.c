/*******************************************************************************
 *
 *   (c) 2016 Ledger
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 ********************************************************************************/

#include <stdbool.h>
#include <string.h>

#include "os.h"
#include "cx.h"
#include "ux.h"
#include "os_io_seproxyhal.h"
#include "globals.h"
#include "mainHelpers.h"

#include "responseCodes.h"

keyDerivationPath_t path;
tx_state_t global_tx_state;

// The expected CLA byte
#define CLA 0xE0

// The Ledger uses APDU commands (https://en.wikipedia.org/wiki/Smart_card_application_protocol_data_unit)
// for performing actions. The INS byte contains the instruction code that determines which action to perform.
#define OFFSET_CLA   0x00
#define OFFSET_INS   0x01
#define OFFSET_P1    0x02
#define OFFSET_P2    0x03
#define OFFSET_LC    0x04
#define OFFSET_CDATA 0x05

// Main entry of application that listens for APDU commands that will be received from the
// computer. The APDU commands control what flow is activated, i.e. which control flow is initiated.
void concordium_main(
    void (*handler)(uint8_t, uint8_t *, uint8_t, uint8_t, uint8_t, volatile unsigned int *, bool),
    void *global_state) {
    volatile unsigned int rx = 0;
    volatile unsigned int tx = 0;
    volatile unsigned int flags = 0;

    for (;;) {
        volatile unsigned short sw = 0;

        BEGIN_TRY {
            TRY {
                rx = tx;
                tx = 0;  // ensure no race in catch_other if io_exchange throws
                         // an error
                rx = io_exchange(CHANNEL_APDU | flags, rx);
                flags = 0;

                // no apdu received, well, reset the session, and reset the
                // bootloader configuration
                if (rx == 0) {
                    THROW(ERROR_NO_APDU_RECEIVED);
                }

                if (G_io_apdu_buffer[OFFSET_CLA] != CLA) {
                    THROW(ERROR_INVALID_CLA);
                }

                uint8_t INS = G_io_apdu_buffer[OFFSET_INS];
                uint8_t p1 = G_io_apdu_buffer[OFFSET_P1];
                uint8_t p2 = G_io_apdu_buffer[OFFSET_P2];
                uint8_t lc = G_io_apdu_buffer[OFFSET_LC];
                uint8_t *cdata = G_io_apdu_buffer + OFFSET_CDATA;

                bool isInitialCall = false;
                if (global_tx_state.currentInstruction == -1) {
                    memset(&global_state, 0, sizeof(global_state));
                    global_tx_state.currentInstruction = INS;
                    isInitialCall = true;
                } else if (global_tx_state.currentInstruction != INS) {
                    // Caller attempted to switch instruction in the middle
                    // of a multi command flow. This is not allowed, as in the
                    // worst case, an attacker could trick a user to sign a mixed
                    // transaction.
                    THROW(ERROR_INVALID_STATE);
                }

                handler(INS, cdata, p1, p2, lc, &flags, isInitialCall);
            }

            CATCH_OTHER(e) {
                switch (e) {
                    case ERROR_NO_APDU_RECEIVED:
                    case ERROR_REJECTED_BY_USER:
                    case ERROR_INVALID_STATE:
                    case ERROR_INVALID_PATH:
                    case ERROR_INVALID_PARAM:
                    case ERROR_INVALID_TRANSACTION:
                    case ERROR_INVALID_INSTRUCTION:
                    case ERROR_FAILED_CX_OPERATION:
                    case ERROR_INVALID_CLA:
                    case ERROR_DEVICE_LOCKED:
                        global_tx_state.currentInstruction = -1;
                        sw = e;
                        G_io_apdu_buffer[tx] = sw >> 8;
                        G_io_apdu_buffer[tx + 1] = sw;
                        tx += 2;
                        break;
                    default:
                        // An unknown error was thrown. Reset the device if
                        // this happens.
                        io_seproxyhal_se_reset();
                        break;
                }
            }
            FINALLY {
            }
        }
        END_TRY;
    }
}

// Required Ledger magic starts here and ends at the end of the file.
// The only way to understand what is happening is to look at the source code in the SDK,
// as there is no help to get anywhere else. The magic has been taken from the helloworld sample application
// provided by Ledger at: https://github.com/LedgerHQ/ledger-sample-apps/blob/master/blue-app-helloworld/src/main.c.
unsigned char G_io_seproxyhal_spi_buffer[IO_SEPROXYHAL_BUFFER_SIZE_B];

void io_seproxyhal_display(const bagl_element_t *element) {
    io_seproxyhal_display_default((bagl_element_t *) element);
}

unsigned char io_event(__attribute__((unused)) unsigned char channel) {
    // can't have more than one tag in the reply, not supported yet.
    switch (G_io_seproxyhal_spi_buffer[0]) {
        case SEPROXYHAL_TAG_FINGER_EVENT:
            UX_FINGER_EVENT(G_io_seproxyhal_spi_buffer);
            break;

        case SEPROXYHAL_TAG_BUTTON_PUSH_EVENT:
            UX_BUTTON_PUSH_EVENT(G_io_seproxyhal_spi_buffer);
            break;

        case SEPROXYHAL_TAG_STATUS_EVENT:
            if (G_io_apdu_media == IO_APDU_MEDIA_USB_HID &&
                !(U4BE(G_io_seproxyhal_spi_buffer, 3) & SEPROXYHAL_TAG_STATUS_EVENT_FLAG_USB_POWERED)) {
                THROW(EXCEPTION_IO_RESET);
            }
            UX_DEFAULT_EVENT();
            break;

        case SEPROXYHAL_TAG_DISPLAY_PROCESSED_EVENT:
            UX_DISPLAYED_EVENT({});
            break;

        case SEPROXYHAL_TAG_TICKER_EVENT:
            UX_TICKER_EVENT(G_io_seproxyhal_spi_buffer, {});
            break;

        default:
            UX_DEFAULT_EVENT();
            break;
    }

    // close the event if not done previously (by a display or whatever)
    if (!io_seproxyhal_spi_is_status_sent()) {
        io_seproxyhal_general_status();
    }

    // command has been processed, DO NOT reset the current APDU transport
    return 1;
}

unsigned short io_exchange_al(unsigned char channel, unsigned short tx_len) {
    switch (channel & ~(IO_FLAGS)) {
        case CHANNEL_KEYBOARD:
            break;
        // multiplexed io exchange over a SPI channel and TLV encapsulated protocol
        case CHANNEL_SPI:
            if (tx_len) {
                io_seproxyhal_spi_send(G_io_apdu_buffer, tx_len);
                if (channel & IO_RESET_AFTER_REPLIED) {
                    reset();
                }
                return 0;  // nothing received from the master so far (it's a tx transaction)
            } else {
                return io_seproxyhal_spi_recv(G_io_apdu_buffer, sizeof(G_io_apdu_buffer), 0);
            }
        default:
            THROW(INVALID_PARAMETER);
    }
    return 0;
}

void app_exit(void) {
    BEGIN_TRY_L(exit) {
        TRY_L(exit) {
            os_sched_exit(-1);
        }
        FINALLY_L(exit) {
        }
    }
    END_TRY_L(exit);
}
