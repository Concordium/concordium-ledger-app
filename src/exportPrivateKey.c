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
static exportPrivateKeyContext_t *ctx = &global.exportPrivateKeyContext;

void exportPrivateKey(void);

UX_STEP_NOCB(
    ux_export_private_key_0_step,
    nn,
    {(char *) global.exportPrivateKeyContext.displayHeader, (char *) global.exportPrivateKeyContext.display});
UX_STEP_CB(ux_export_private_key_accept_step, pb, exportPrivateKey(), {&C_icon_validate_14, "Accept"});
UX_STEP_CB(ux_export_private_key_decline_step, pb, sendUserRejection(), {&C_icon_crossmark, "Decline"});
UX_FLOW(
    ux_export_private_key,
    &ux_export_private_key_0_step,
    &ux_export_private_key_accept_step,
    &ux_export_private_key_decline_step);

#define ID_CRED_SEC 0
#define PRF_KEY     1

#define pathLength 6

void exportPrivateKeySeed(void) {
    cx_ecfp_private_key_t privateKey;
    BEGIN_TRY {
        TRY {
            ctx->path[5] = PRF_KEY | HARDENED_OFFSET;
            getPrivateKey(ctx->path, pathLength, &privateKey);
            uint8_t tx = 0;
            for (int i = 0; i < 32; i++) {
                G_io_apdu_buffer[tx++] = privateKey.d[i];
            }

            if (ctx->exportBoth) {
                ctx->path[5] = ID_CRED_SEC | HARDENED_OFFSET;
                getPrivateKey(ctx->path, pathLength, &privateKey);
                for (int i = 0; i < 32; i++) {
                    G_io_apdu_buffer[tx++] = privateKey.d[i];
                }
            }

            sendSuccess(tx);
        }
        FINALLY {
            explicit_bzero(&privateKey, sizeof(privateKey));
        }
    }
    END_TRY;
}

void exportPrivateKeyBls(void) {
    uint8_t privateKey[32];
    BEGIN_TRY {
        TRY {
            ctx->path[5] = PRF_KEY | HARDENED_OFFSET;
            getBlsPrivateKey(ctx->path, pathLength, privateKey, sizeof(privateKey));
            uint8_t tx = 0;
            memmove(G_io_apdu_buffer, privateKey, sizeof(privateKey));
            tx += sizeof(privateKey);

            if (ctx->exportBoth) {
                ctx->path[5] = ID_CRED_SEC | HARDENED_OFFSET;
                getBlsPrivateKey(ctx->path, pathLength, privateKey, sizeof(privateKey));
                memmove(G_io_apdu_buffer + tx, privateKey, sizeof(privateKey));
                tx += sizeof(privateKey);
            }

            sendSuccess(tx);
        }
        FINALLY {
            explicit_bzero(&privateKey, sizeof(privateKey));
        }
    }
    END_TRY;
}

void exportPrivateKey(void) {
    if (ctx->exportSeed) {
        exportPrivateKeySeed();
    } else {
        exportPrivateKeyBls();
    }
}

#define ACCOUNT_SUBTREE 0
#define NORMAL_ACCOUNTS 0

// Export the PRF key
#define P1_PRF_KEY          0x00
#define P1_PRF_KEY_RECOVERY 0x01
// Export the PRF key and the IdCredSec
#define P1_BOTH 0x02

// Export seeds (Deprecated)
#define P2_SEED 0x01
// Export the BLS keys
#define P2_KEY 0x02

void handleExportPrivateKey(uint8_t *dataBuffer, uint8_t p1, uint8_t p2, volatile unsigned int *flags) {
    if ((p1 != P1_BOTH && p1 != P1_PRF_KEY && p1 != P1_PRF_KEY_RECOVERY) || (p2 != P2_KEY && p2 != P2_SEED)) {
        THROW(ERROR_INVALID_PARAM);
    }

    uint32_t identity = U4BE(dataBuffer, 0);
    uint32_t keyDerivationPath[5] = {
        CONCORDIUM_PURPOSE | HARDENED_OFFSET,
        CONCORDIUM_COIN_TYPE | HARDENED_OFFSET,
        ACCOUNT_SUBTREE | HARDENED_OFFSET,
        NORMAL_ACCOUNTS | HARDENED_OFFSET,
        identity | HARDENED_OFFSET};
    memmove(ctx->path, keyDerivationPath, sizeof(keyDerivationPath));

    ctx->exportBoth = p1 == P1_BOTH;
    ctx->exportSeed = p2 == P2_SEED;

    memmove(ctx->display, "ID #", 4);
    bin2dec(ctx->display + 4, sizeof(ctx->display) - 4, identity);

    if (p1 == P1_BOTH) {
        memmove(ctx->displayHeader, "Create credential", 18);
    } else if (p1 == P1_PRF_KEY_RECOVERY) {
        memmove(ctx->displayHeader, "Recover credentials", 20);
    } else if (p1 == P1_PRF_KEY) {
        memmove(ctx->displayHeader, "Decrypt", 8);
    }

    ux_flow_init(0, ux_export_private_key, NULL);
    *flags |= IO_ASYNCH_REPLY;
}
