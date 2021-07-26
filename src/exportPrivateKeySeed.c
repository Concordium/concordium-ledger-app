#include <os.h>
#include "cx.h"
#include <stdint.h>
#include "util.h"
#include "globals.h"
#include <string.h>
#include "responseCodes.h"

// This class allows for the export of a number of very specific private keys. These private keys are made
// exportable as they are used in computations that are not feasible to carry out on the Ledger device.
// The key derivation paths that are allowed are restricted so that it is not possible to export
// keys that are used for signing.
static const uint32_t HARDENED_OFFSET = 0x80000000;
static exportPrivateKeySeedContext_t *ctx = &global.exportPrivateKeySeedContext;

void exportPrivateKey();

UX_STEP_CB(
    ux_export_private_key_0_step,
    bnnn_paging,
    exportPrivateKey(),
    {
      .title = (char *) global.exportPrivateKeySeedContext.type,
      .text = (char *) global.exportPrivateKeySeedContext.display
    });
UX_FLOW(ux_export_private_key,
    &ux_export_private_key_0_step
);

void exportPrivateKey() {
    cx_ecfp_private_key_t privateKey;
    BEGIN_TRY {
        TRY {
            if (ctx->pathLength == 6) {
                getPrivateKey(ctx->path, ctx->pathLength, &privateKey);
            } else {
                THROW(ERROR_INVALID_PATH);
            }
            uint8_t tx = 0;
            for (int i = 0; i < 32; i++) {
                G_io_apdu_buffer[tx++] = privateKey.d[i];
            }
            sendSuccess(tx);
        }
        FINALLY {
            explicit_bzero(&privateKey, sizeof(privateKey));
        }
    }
    END_TRY;
}

const char* getPrivateKeyTypeName(uint8_t type) {
    switch (type) {
        case 0: return "IdCredSec";
        case 1: return "PRF key";
        default: THROW(ERROR_INVALID_PARAM);
    }
}

#define ACCOUNT_SUBTREE 0
#define NORMAL_ACCOUNTS 0

#define P1_ID_CRED_SEC          0x00
#define P1_PRF_KEY              0x01

void handleExportPrivateKeySeed(uint8_t *dataBuffer, uint8_t p1, volatile unsigned int *flags) {
    memmove(ctx->type, "Export ", 7);
    memmove(ctx->type + 7, getPrivateKeyTypeName(p1), 12);
    
    if (p1 == P1_ID_CRED_SEC || p1 == P1_PRF_KEY) {
        uint32_t identity = U4BE(dataBuffer, 0);
        uint8_t typeOfKey = p1 == P1_ID_CRED_SEC ? 0 : 1;
        uint32_t keyDerivationPath[] = {
            CONCORDIUM_PURPOSE | HARDENED_OFFSET,
            CONCORDIUM_COIN_TYPE | HARDENED_OFFSET,
            ACCOUNT_SUBTREE | HARDENED_OFFSET,
            NORMAL_ACCOUNTS | HARDENED_OFFSET,
            identity | HARDENED_OFFSET,
            typeOfKey | HARDENED_OFFSET
        };
        memmove(ctx->path, keyDerivationPath, sizeof(keyDerivationPath) * 6);
        ctx->pathLength = 6;

        memmove(ctx->display, "ID #", 4);
        bin2dec(ctx->display + 4, identity);
    } else {
        THROW(ERROR_INVALID_PARAM);
    }

    ux_flow_init(0, ux_export_private_key, NULL);
    *flags |= IO_ASYNCH_REPLY;
}
