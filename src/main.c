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

#include "getPublicKey.h"
#include "glyphs.h"
#include "menu.h"
#include "os.h"
#include "signTransaction.h"
#include "ux.h"
#include <string.h>

// The expected CLA byte
#define CLA 0xE0

// The Ledger uses APDU commands (https://en.wikipedia.org/wiki/Smart_card_application_protocol_data_unit)
// for performing actions. The INS byte contains the instruction code that determines which action to perform.
#define OFFSET_CLA 0x00
#define OFFSET_INS 0x01
#define OFFSET_LC    0x04
#define OFFSET_CDATA 0x05

// An INS instruction containing 0x01 means that we should start the public-key flow.
#define INS_GET_PUBLIC_KEY 0x01

// An INS instruction containing 0x02 means that we should start the signing flow.
#define INS_SIGN_TRANSACTION 0x02

// Main entry of application that listens for APDU commands that will be received from the
// computer. The APDU commands control what flow is activated, i.e. which control flow is initiated.
static void concordium_main(void) {
    volatile unsigned int rx = 0;
    volatile unsigned int tx = 0;
    volatile unsigned int flags = 0;

    for (;;) {
        volatile unsigned short sw = 0;

        BEGIN_TRY {
            TRY {
                rx = tx;
                tx = 0; // ensure no race in catch_other if io_exchange throws
                        // an error
                rx = io_exchange(CHANNEL_APDU | flags, rx);
                flags = 0;

                // no apdu received, well, reset the session, and reset the
                // bootloader configuration
                if (rx == 0) {
                    THROW(0x6982);
                }

                if (G_io_apdu_buffer[OFFSET_CLA] != CLA) {
                    THROW(0x6E00);
                }

                switch (G_io_apdu_buffer[OFFSET_INS]) {
                    case INS_GET_PUBLIC_KEY:
                        handleGetPublicKey(G_io_apdu_buffer + OFFSET_CDATA, &flags);
                    case INS_SIGN_TRANSACTION:
                        signTransaction(G_io_apdu_buffer + OFFSET_CDATA, G_io_apdu_buffer[OFFSET_LC], &flags);
                    default:
                        THROW(0x6D00);
                        break;
                }
            }
            CATCH_OTHER(e) {
                switch (e & 0xF000) {
                case 0x6000:
                case 0x9000:
                    sw = e;
                    break;
                default:
                    sw = 0x6800 | (e & 0x7FF);
                    break;
                }
                // Unexpected exception => report
                G_io_apdu_buffer[tx] = sw >> 8;
                G_io_apdu_buffer[tx + 1] = sw;
                tx += 2;
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
    io_seproxyhal_display_default((bagl_element_t *)element);
}

unsigned char io_event(unsigned char channel) {
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
			!(U4BE(G_io_seproxyhal_spi_buffer, 3) &
			  SEPROXYHAL_TAG_STATUS_EVENT_FLAG_USB_POWERED)) {
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
			return 0; // nothing received from the master so far (it's a tx transaction)
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

// Boot procedure that ensures that the main menu is loaded, and that the application then starts
// listening for APDU commands by running concordium_main().
__attribute__((section(".boot"))) int main(void) {
    // exit critical section
    __asm volatile("cpsie i");

    os_boot();

    for (;;) {
        UX_INIT();

        BEGIN_TRY {
            TRY {
                io_seproxyhal_init();
                USB_power(0);
                USB_power(1);
                ui_idle();
                concordium_main();
            }
            CATCH(EXCEPTION_IO_RESET) {
                // reset IO and UX before continuing
                continue;
            }
            CATCH_ALL {
                break;
            }
            FINALLY {
            }
        }
        END_TRY;
    }
    app_exit();
    return 0;
}