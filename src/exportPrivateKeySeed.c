#include <os.h>
#include "cx.h"
#include <stdint.h>
#include "util.h"
#include "globals.h"
#include <string.h>

// This class allows for the export of a number of very specific private keys. These private keys are made
// exportable as they are used in computations that are not feasible to carry out on the Ledger device.
// The key derivation paths that are allowed are restricted so that it is not possible to export
// keys that are used for signing.
static const uint32_t HARDENED_OFFSET = 0x80000000;
static exportPrivateKeySeedContext_t *ctx = &global.exportPrivateKeySeedContext;

void exportPrivateKey();

UX_STEP_CB(
    ux_export_private_key_0_step,
    bn_paging,
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
                getPrivateKey(ctx->arPath, ctx->pathLength, &privateKey);
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
        case 2: return "AR dec. key";
        default: THROW(SW_INVALID_PARAM);
    }
}

#define ACCOUNT_SUBTREE 0
#define NORMAL_ACCOUNTS 0
#define AR_SUBTREE      1

#define P1_ID_CRED_SEC          0x00    // Get ID_cred_sec private key seed.
#define P1_PRF_KEY              0x01    // Get PRF key.
#define P1_AR_DECRYPTION_KEY    0x02    // Get anonymity revocation decryption key.

void handleExportPrivateKeySeed(uint8_t *dataBuffer, uint8_t p1, volatile unsigned int *flags) {
    os_memmove(ctx->type, "Export ", 7);
    os_memmove(ctx->type + 7, getPrivateKeyTypeName(p1), 12);
    
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
        os_memmove(ctx->path, keyDerivationPath, sizeof(keyDerivationPath) * 6);
        ctx->pathLength = 6;

        os_memmove(ctx->display, "ID #", 4);
        bin2dec(ctx->display + 4, identity);
    } else if (p1 == P1_AR_DECRYPTION_KEY) {
        uint32_t idp = U4BE(dataBuffer, 0);
        uint32_t ar_index = U4BE(dataBuffer, 4);
        uint32_t keyDerivationPath[] = {
            CONCORDIUM_PURPOSE | HARDENED_OFFSET,
            CONCORDIUM_COIN_TYPE | HARDENED_OFFSET,
            AR_SUBTREE | HARDENED_OFFSET,
            idp | HARDENED_OFFSET,
            ar_index | HARDENED_OFFSET,
        };
        os_memmove(ctx->arPath, keyDerivationPath, sizeof(keyDerivationPath) * 5);
        ctx->pathLength = 5;

        uint8_t lengthCount = 5;
        os_memmove(ctx->display, "IDP #", lengthCount);
        lengthCount += bin2dec(ctx->display + lengthCount, idp);
        os_memmove(ctx->display + lengthCount, " ", 1);
        lengthCount += 1;
        os_memmove(ctx->display + lengthCount, "AR #", 4);
        lengthCount += 4;
        bin2dec(ctx->display + lengthCount, ar_index);
    } else {
        THROW(SW_INVALID_PARAM);
    }

    ux_flow_init(0, ux_export_private_key, NULL);
    *flags |= IO_ASYNCH_REPLY;
}
