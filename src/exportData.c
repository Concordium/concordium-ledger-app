#include <os.h>
#include <stdint.h>
#include <string.h>

#include "cx.h"
#include "globals.h"
#include "responseCodes.h"
#include "util.h"
#include "ux.h"

// This class allows for the export of a number of very specific private keys. These private keys are made
// exportable as they are used in computations that are not feasible to carry out on the Ledger device.
// The key derivation paths that are allowed are restricted so that it is not possible to export
// keys that are used for signing.
static const uint32_t HARDENED_OFFSET = 0x80000000;
static exportDataContext_t *ctx = &global.exportDataContext;

void exportData(void);

UX_STEP_NOCB(
    ux_export_data_0_step,
    nn,
    {(char *) global.exportDataContext.displayHeader, (char *) global.exportDataContext.display});
UX_STEP_CB(ux_export_data_accept_step, pb, exportData(), {&C_icon_validate_14, "Accept"});
UX_STEP_CB(ux_export_data_decline_step, pb, sendUserRejection(), {&C_icon_crossmark, "Decline"});
UX_FLOW(ux_export_data, &ux_export_data_0_step, &ux_export_data_accept_step, &ux_export_data_decline_step);

#define P1_ALLOW_ALL     0x00
#define P1_ID_CRED_SEC   0x02
#define P1_PRF_KEY       0x03
#define P1_BLINDING_RND  0x04
#define P1_ATTRIBUTE_RND 0x05

// Export the BLS keys (For mainnet)
#define P2_KEY_MAINNET 0x00
// Export the BLS keys (For testnet)
#define P2_KEY_TESTNET 0x01

void exportData(void) {
    if (ctx->p1 == P1_ALLOW_ALL) {
        ctx->acceptedAll = !ctx->acceptedAll;
        if (ctx->acceptedAll) {
            sendSuccessNoIdle();
        } else {
            sendSuccess(0);
        }
        return;
    }

    uint8_t privateKey[32];
    BEGIN_TRY {
        TRY {
            ctx->path[4] = ctx->p1 | HARDENED_OFFSET;
            getBlsPrivateKey(ctx->path, ctx->pathLength, privateKey, sizeof(privateKey));
            uint8_t tx = 0;
            memmove(G_io_apdu_buffer, privateKey, sizeof(privateKey));
            tx += sizeof(privateKey);
            if (ctx->acceptedAll) {
                sendSuccessResultNoIdle(tx);
            } else {
                sendSuccess(tx);
            }
        }
        FINALLY {
            explicit_bzero(&privateKey, sizeof(privateKey));
        }
    }
    END_TRY;
}

void handleExportData(uint8_t *dataBuffer, uint8_t p1, uint8_t p2, volatile unsigned int *flags, bool isInitialCall) {
    switch (p1) {
        case P1_ALLOW_ALL:
        case P1_ID_CRED_SEC:
        case P1_PRF_KEY:
        case P1_BLINDING_RND:
        case P1_ATTRIBUTE_RND:
            break;
        default:
            THROW(ERROR_INVALID_PARAM);
    }

    if (isInitialCall) {
        ctx->acceptedAll = false;
    }

    uint32_t coinType;
    if (p2 == P2_KEY_MAINNET) {
        coinType = CONCORDIUM_COIN_TYPE_MAINNET;
    } else if (p2 == P2_KEY_TESTNET) {
        coinType = CONCORDIUM_COIN_TYPE_TESTNET;
    } else {
        THROW(ERROR_INVALID_PARAM);
    }

    ctx->p1 = p1;

    uint32_t identityProvider = U4BE(dataBuffer, 0);
    dataBuffer += 4;
    uint32_t identity = U4BE(dataBuffer, 0);
    dataBuffer += 4;

    // If we are accepted all, we need to confirm that the coinType/identityProvider/identity has not been changed
    if (ctx->acceptedAll &&
        (ctx->path[1] != (coinType | HARDENED_OFFSET) || ctx->path[2] != (identityProvider | HARDENED_OFFSET) ||
         ctx->path[3] != (identity | HARDENED_OFFSET))) {
        THROW(ERROR_INVALID_PARAM);
    }

    uint32_t keyDerivationPath[4] = {
        CONCORDIUM_PURPOSE | HARDENED_OFFSET,
        coinType | HARDENED_OFFSET,
        identityProvider | HARDENED_OFFSET,
        identity | HARDENED_OFFSET,
    };
    memmove(ctx->path, keyDerivationPath, sizeof(keyDerivationPath));

    if (p1 == P1_ATTRIBUTE_RND) {
        uint32_t cred_counter = U4BE(dataBuffer, 0);
        ctx->path[5] = cred_counter | HARDENED_OFFSET;
        dataBuffer += 4;
        uint32_t attribute = U4BE(dataBuffer, 0);
        ctx->path[6] = attribute | HARDENED_OFFSET;
        ctx->pathLength = 7;
    } else {
        ctx->pathLength = 5;
    }

    if (ctx->acceptedAll) {
        exportData();
    } else {
        memmove(ctx->display, "ID #", 4);
        bin2dec(ctx->display + 4, sizeof(ctx->display) - 4, identity);

        if (p1 == P1_ALLOW_ALL) {
            memmove(ctx->displayHeader, "Export identity data", 21);
        } else if (p1 == P1_PRF_KEY) {
            memmove(ctx->displayHeader, "Decrypt", 8);
        } else if (p1 == P1_ID_CRED_SEC) {
            memmove(ctx->displayHeader, "Export IdCredSec", 17);
        } else if (p1 == P1_BLINDING_RND || p1 == P1_ATTRIBUTE_RND) {
            memmove(ctx->displayHeader, "Export Randomness", 18);
        }

        ux_flow_init(0, ux_export_data, NULL);
        *flags |= IO_ASYNCH_REPLY;
    }
}
