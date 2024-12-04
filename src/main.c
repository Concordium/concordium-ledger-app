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
#include "mainHelpers.h"
#include "exportPrivateKey.h"
#include "getPublicKey.h"
#include "globals.h"
#include "glyphs.h"
#include "menu.h"
#include "os_io_seproxyhal.h"
#include "responseCodes.h"
#include "seproxyhal_protocol.h"
#include "signConfigureBaker.h"
#include "signConfigureDelegation.h"
#include "signCredentialDeployment.h"
#include "signEncryptedAmountTransfer.h"
#include "signPublicInformationForIp.h"
#include "signTransfer.h"
#include "signTransferToEncrypted.h"
#include "signTransferToPublic.h"
#include "signTransferWithSchedule.h"
#include "ux.h"
#include "verifyAddress.h"
#include "getAppName.h"
// Global variable definitions
instructionContext global;
accountSender_t global_account_sender;

#define INS_VERIFY_ADDRESS 0x00

// An INS instruction containing 0x01 means that we should start the public-key flow.
#define INS_GET_PUBLIC_KEY 0x01

// An INS instruction containing 0x02 means that we should start the transfer signing flow.
#define INS_SIGN_TRANSFER 0x02

// An INS instruction containing 0x03 means that we should start the scheduled transfer signing
// flow.
#define INS_SIGN_TRANSFER_WITH_SCHEDULE 0x03

// An INS instruction containing 0x04 means that we should start the credential deployment signing
// flow.
#define INS_CREDENTIAL_DEPLOYMENT 0x04

#define INS_EXPORT_PRIVATE_KEY 0x05

#define INS_ENCRYPTED_AMOUNT_TRANSFER 0x10
#define INS_TRANSFER_TO_ENCRYPTED     0x11
#define INS_TRANSFER_TO_PUBLIC        0x12

#define INS_CONFIGURE_DELEGATION 0x17
#define INS_CONFIGURE_BAKER      0x18

#define INS_PUBLIC_INFO_FOR_IP 0x20

#define INS_SIGN_UPDATE_CREDENTIAL 0x31

#define INS_SIGN_TRANSFER_WITH_MEMO              0x32
#define INS_ENCRYPTED_AMOUNT_TRANSFER_WITH_MEMO  0x33
#define INS_SIGN_TRANSFER_WITH_SCHEDULE_AND_MEMO 0x34
#define INS_REGISTER_DATA                        0x35

#define INS_APP_NAME     0x36
#define INS_GET_APP_NAME 0x21

void handler(uint8_t INS,
             uint8_t *cdata,
             uint8_t p1,
             uint8_t p2,
             uint8_t lc,
             volatile unsigned int *flags,
             bool isInitialCall) {
    switch (INS) {
        case INS_GET_PUBLIC_KEY:
            handleGetPublicKey(cdata, p1, p2, flags);
            break;
        case INS_VERIFY_ADDRESS:
            handleVerifyAddress(cdata, p1, flags);
            break;
        case INS_SIGN_TRANSFER:
            handleSignTransfer(cdata, flags);
            break;
        case INS_SIGN_TRANSFER_WITH_MEMO:
            handleSignTransferWithMemo(cdata, p1, lc, flags, isInitialCall);
            break;
        case INS_SIGN_TRANSFER_WITH_SCHEDULE:
            handleSignTransferWithSchedule(cdata, p1, flags, isInitialCall);
            break;
        case INS_SIGN_TRANSFER_WITH_SCHEDULE_AND_MEMO:
            handleSignTransferWithScheduleAndMemo(cdata, p1, lc, flags, isInitialCall);
            break;
        case INS_CREDENTIAL_DEPLOYMENT:
            handleSignCredentialDeployment(cdata, p1, p2, flags, isInitialCall);
            break;
        case INS_EXPORT_PRIVATE_KEY:
            handleExportPrivateKey(cdata, p1, p2, flags);
            break;
        case INS_TRANSFER_TO_ENCRYPTED:
            handleSignTransferToEncrypted(cdata, flags);
            break;
        case INS_ENCRYPTED_AMOUNT_TRANSFER:
            handleSignEncryptedAmountTransfer(cdata, p1, lc, flags, isInitialCall);
            break;
        case INS_ENCRYPTED_AMOUNT_TRANSFER_WITH_MEMO:
            handleSignEncryptedAmountTransferWithMemo(cdata, p1, lc, flags, isInitialCall);
            break;
        case INS_TRANSFER_TO_PUBLIC:
            handleSignTransferToPublic(cdata, p1, lc, flags, isInitialCall);
            break;
        case INS_REGISTER_DATA:
            handleSignRegisterData(cdata, p1, lc, flags, isInitialCall);
            break;
        case INS_PUBLIC_INFO_FOR_IP:
            handleSignPublicInformationForIp(cdata, p1, flags, isInitialCall);
            break;
        case INS_CONFIGURE_BAKER:
            handleSignConfigureBaker(cdata, p1, lc, flags, isInitialCall);
            break;
        case INS_CONFIGURE_DELEGATION:
            handleSignConfigureDelegation(cdata, lc, flags);
            break;
        case INS_SIGN_UPDATE_CREDENTIAL:
            handleSignUpdateCredential(cdata, p1, p2, flags, isInitialCall);
            break;
        case INS_GET_APP_NAME:
            handleGetAppName(flags);
            break;
        default:
            THROW(ERROR_INVALID_INSTRUCTION);
            break;
    }
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

#if defined(HAVE_BLE)
                G_io_app.plane_mode = os_setting_get(OS_SETTING_PLANEMODE, NULL, 0);
#endif

                USB_power(0);
                USB_power(1);
                ui_menu_main();

#if defined(HAVE_BLE)
                BLE_power(0, NULL);
                BLE_power(1, NULL);
#endif

                concordium_main(&handler, &global);
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
