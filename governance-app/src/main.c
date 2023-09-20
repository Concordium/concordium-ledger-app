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
#include "mainHelpers.h"
#include "cx.h"
#include "globals.h"
#include "glyphs.h"
#include "menu.h"
#include "os_io_seproxyhal.h"
#include "responseCodes.h"
#include "seproxyhal_protocol.h"
#include "signHigherLevelKeyUpdate.h"
#include "signUpdateAuthorizations.h"
#include "signUpdateBakerStakeThreshold.h"
#include "signUpdateElectionDifficulty.h"
#include "signUpdateExchangeRate.h"
#include "signUpdateFoundationAccount.h"
#include "signUpdateGasRewards.h"
#include "signUpdateMintDistribution.h"
#include "signUpdateProtocol.h"
#include "signUpdateTransactionFeeDistribution.h"
#include "signUpdateTimeParameters.h"
#include "signUpdateCooldownParameters.h"
#include "signUpdatePoolParameters.h"
#include "signUpdateTimeoutParameters.h"
#include "signUpdateMinBlockTime.h"
#include "signUpdateBlockEnergyLimit.h"
#include "signUpdateFinalizationCommitteeParameters.h"
#include "ux.h"

// Global variable definitions
instructionContext global;

// An INS instruction containing 0x01 means that we should start the public-key flow.
#define INS_GET_PUBLIC_KEY 0x01

#define INS_EXPORT_PRIVATE_KEY   0x05
#define INS_UPDATE_EXCHANGE_RATE 0x06

#define INS_UPDATE_PROTOCOL              0x21
#define INS_UPDATE_TRANSACTION_FEE_DIST  0x22
#define INS_UPDATE_GAS_REWARDS           0x23
#define INS_UPDATE_FOUNDATION_ACCOUNT    0x24
#define INS_UPDATE_MINT_DISTRIBUTION     0x25
#define INS_UPDATE_ELECTION_DIFFICULTY   0x26
#define INS_UPDATE_BAKER_STAKE_THRESHOLD 0x27

#define INS_UPDATE_ROOT_KEYS          0x28
#define INS_UPDATE_LEVEL1_KEYS        0x29
#define INS_UPDATE_LEVEL2_KEYS_ROOT   0x2A
#define INS_UPDATE_LEVEL2_KEYS_LEVEL1 0x2B
#define INS_ADD_ANONYMITY_REVOKER     0x2C
#define INS_ADD_IDENTITY_PROVIDER     0x2D

#define INS_UPDATE_COOLDOWN_PARAMETERS 0x40
#define INS_UPDATE_POOL_PARAMETERS     0x41
#define INS_UPDATE_TIME_PARAMETERS     0x42

// Protocol update 6 transactions.
#define INS_UPDATE_TIMEOUT_PARAMETERS 0x43
#define INS_UPDATE_MIN_BLOCK_TIME     0x44
#define INS_UPDATE_BLOCK_ENERGY_LIMIT 0x45
#define INS_UPDATE_FINALIZATION_COMMITTEE_PARAMETERS 0x46

void handler(
    uint8_t INS,
    uint8_t *cdata,
    uint8_t p1,
    uint8_t p2,
    uint8_t lc,
    volatile unsigned int *flags,
    bool isInitialCall
    ) {
                switch (INS) {
                    case INS_GET_PUBLIC_KEY:
                        handleGetPublicKey(cdata, p1, p2, flags);
                        break;
                    case INS_UPDATE_EXCHANGE_RATE:
                        handleSignUpdateExchangeRate(cdata, flags);
                        break;
                    case INS_UPDATE_PROTOCOL:
                        handleSignUpdateProtocol(cdata, p1, lc, flags, isInitialCall);
                        break;
                    case INS_UPDATE_TRANSACTION_FEE_DIST:
                        handleSignUpdateTransactionFeeDistribution(cdata, flags);
                        break;
                    case INS_UPDATE_GAS_REWARDS:
                        handleSignUpdateGasRewards(cdata, flags);
                        break;
                    case INS_UPDATE_FOUNDATION_ACCOUNT:
                        handleSignUpdateFoundationAccount(cdata, flags);
                        break;
                    case INS_UPDATE_MINT_DISTRIBUTION:
                        handleSignUpdateMintDistribution(cdata, p2, flags);
                        break;
                    case INS_UPDATE_ELECTION_DIFFICULTY:
                        handleSignUpdateElectionDifficulty(cdata, flags);
                        break;
                    case INS_UPDATE_BAKER_STAKE_THRESHOLD:
                        handleSignUpdateBakerStakeThreshold(cdata, flags);
                        break;
                    case INS_UPDATE_ROOT_KEYS:
                        handleSignHigherLevelKeys(cdata, p1, UPDATE_TYPE_UPDATE_ROOT_KEYS, flags, isInitialCall);
                        break;
                    case INS_UPDATE_LEVEL1_KEYS:
                        handleSignHigherLevelKeys(cdata, p1, UPDATE_TYPE_UPDATE_LEVEL1_KEYS, flags, isInitialCall);
                        break;
                    case INS_UPDATE_LEVEL2_KEYS_ROOT:
                        handleSignUpdateAuthorizations(
                            cdata,
                            p1,
                            p2,
                            UPDATE_TYPE_UPDATE_ROOT_KEYS,
                            lc,
                            flags,
                            isInitialCall);
                        break;
                    case INS_UPDATE_LEVEL2_KEYS_LEVEL1:
                        handleSignUpdateAuthorizations(
                            cdata,
                            p1,
                            p2,
                            UPDATE_TYPE_UPDATE_LEVEL1_KEYS,
                            lc,
                            flags,
                            isInitialCall);
                        break;
                    case INS_ADD_IDENTITY_PROVIDER:
                        handleSignAddIdentityProvider(cdata, p1, lc, flags, isInitialCall);
                        break;
                    case INS_ADD_ANONYMITY_REVOKER:
                        handleSignAddAnonymityRevoker(cdata, p1, lc, flags, isInitialCall);
                        break;
                    case INS_UPDATE_TIME_PARAMETERS:
                        handleSignUpdateTimeParameters(cdata, flags);
                        break;
                    case INS_UPDATE_COOLDOWN_PARAMETERS:
                        handleSignUpdateCooldownParameters(cdata, flags);
                        break;
                    case INS_UPDATE_POOL_PARAMETERS:
                        handleSignUpdatePoolParameters(cdata, p1, flags, isInitialCall);
                        break;
                    case INS_UPDATE_TIMEOUT_PARAMETERS:
                        handleSignTimeoutParameters(cdata, flags);
                        break;
                    case INS_UPDATE_MIN_BLOCK_TIME:
                        handleSignMinBlockTime(cdata, flags);
                        break;
                    case INS_UPDATE_BLOCK_ENERGY_LIMIT:
                        handleSignUpdateBlockEnergyLimit(cdata, flags);
                        break;
                    case INS_UPDATE_FINALIZATION_COMMITTEE_PARAMETERS:
                        handleSignUpdateFinalizationCommitteeParameters(cdata, flags);
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
                ui_idle();

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
