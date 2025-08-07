#include "globals.h"

// This class allows for the export of a number of very specific private keys. These private keys
// are made exportable as they are used in computations that are not feasible to carry out on the
// Ledger device. The key derivation paths that are allowed are restricted so that it is not
// possible to export keys that are used for signing.
static const uint32_t HARDENED_OFFSET = 0x80000000;
static exportPrivateKeyContext_t *ctx = &global.exportPrivateKeyContext;

void exportPrivateKeySeed(void) {
    cx_ecfp_private_key_t privateKey;
    BEGIN_TRY {
        TRY {
            uint8_t lastSubPath;
            uint8_t lastSubPathIndex;
            if (ctx->isNewPath) {
                lastSubPath = NEW_PRF_KEY;
                lastSubPathIndex = 4;
            } else {
                lastSubPath = LEGACY_PRF_KEY;
                lastSubPathIndex = 5;
            }
            ctx->path[lastSubPathIndex] = lastSubPath | HARDENED_OFFSET;
            getPrivateKey(ctx->path, lastSubPathIndex + 1, &privateKey);
            uint8_t tx = 0;
            for (int i = 0; i < 32; i++) {
                G_io_apdu_buffer[tx++] = privateKey.d[i];
            }

            if (ctx->exportBoth) {
                if (ctx->isNewPath) {
                    lastSubPath = NEW_ID_CRED_SEC;
                } else {
                    lastSubPath = LEGACY_ID_CRED_SEC;
                }
                ctx->path[lastSubPathIndex] = lastSubPath | HARDENED_OFFSET;
                getPrivateKey(ctx->path, lastSubPathIndex + 1, &privateKey);
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
            uint8_t lastSubPath;
            uint8_t lastSubPathIndex;
            if (ctx->isNewPath) {
                lastSubPath = NEW_PRF_KEY;
                lastSubPathIndex = 4;
            } else {
                lastSubPath = LEGACY_PRF_KEY;
                lastSubPathIndex = 5;
            }
            ctx->path[lastSubPathIndex] = lastSubPath | HARDENED_OFFSET;
            getBlsPrivateKey(ctx->path, lastSubPathIndex + 1, privateKey, sizeof(privateKey));
            uint8_t tx = 0;
            if (sizeof(privateKey) > sizeof(G_io_apdu_buffer)) {
                THROW(ERROR_BUFFER_OVERFLOW);
            }
            memmove(G_io_apdu_buffer, privateKey, sizeof(privateKey));
            tx += sizeof(privateKey);

            if (ctx->exportBoth) {
                if (ctx->isNewPath) {
                    lastSubPath = NEW_ID_CRED_SEC;
                } else {
                    lastSubPath = LEGACY_ID_CRED_SEC;
                }
                ctx->path[lastSubPathIndex] = lastSubPath | HARDENED_OFFSET;
                getBlsPrivateKey(ctx->path, lastSubPathIndex + 1, privateKey, sizeof(privateKey));
                if (sizeof(privateKey) + tx > sizeof(G_io_apdu_buffer)) {
                    THROW(ERROR_BUFFER_OVERFLOW);
                }
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

void handleExportPrivateKey(uint8_t *dataBuffer,
                            uint8_t p1,
                            uint8_t p2,
                            uint8_t lc,
                            bool legacyDerivationPath,
                            volatile unsigned int *flags) {
    if ((p1 != P1_BOTH && p1 != P1_PRF_KEY && p1 != P1_PRF_KEY_RECOVERY) ||
        (p2 != P2_KEY && p2 != P2_SEED)) {
        THROW(ERROR_INVALID_PARAM);
    }
    size_t offset = 0;

    ctx->isNewPath = !legacyDerivationPath;
    uint8_t remainingDataLength = lc - offset;
    uint32_t identity_provider;
    uint32_t identity;
    if (ctx->isNewPath) {
        if (remainingDataLength < 4) {
            THROW(ERROR_INVALID_PATH);
        }
        identity_provider = U4BE(dataBuffer, offset);
        offset += 4;
        remainingDataLength -= 4;
    }
    if (remainingDataLength < 4) {
        THROW(ERROR_INVALID_PATH);
    }
    identity = U4BE(dataBuffer, offset);
    uint32_t *keyDerivationPath;
    size_t pathLength;
    if (ctx->isNewPath) {
        keyDerivationPath = (uint32_t[4]){NEW_PURPOSE | HARDENED_OFFSET,
                                          NEW_COIN_TYPE | HARDENED_OFFSET,
                                          identity_provider | HARDENED_OFFSET,
                                          identity | HARDENED_OFFSET};
        pathLength = 4;
    } else {
        keyDerivationPath = (uint32_t[5]){LEGACY_PURPOSE | HARDENED_OFFSET,
                                          LEGACY_COIN_TYPE | HARDENED_OFFSET,
                                          ACCOUNT_SUBTREE | HARDENED_OFFSET,
                                          NORMAL_ACCOUNTS | HARDENED_OFFSET,
                                          identity | HARDENED_OFFSET};
        pathLength = 5;
    }
    memmove(ctx->path, keyDerivationPath, pathLength * sizeof(uint32_t));
    ctx->pathLength = pathLength * sizeof(uint32_t);

    ctx->exportBoth = p1 == P1_BOTH;
    ctx->exportSeed = p2 == P2_SEED;

    // Reset the offset to 0
    offset = 0;
    if (ctx->isNewPath) {
        memmove(ctx->display, "IDP#", 4);
        offset += 4;
        offset += bin2dec(ctx->display + offset, sizeof(ctx->display) - offset, identity_provider);
        // Remove the null terminator
        offset -= 1;
    }

    memmove(ctx->display + offset, " ID#", 4);
    offset += 4;
    bin2dec(ctx->display + offset, sizeof(ctx->display) - offset, identity);

    if (p1 == P1_BOTH) {
        memmove(ctx->displayHeader, "Create credential", 18);
    } else if (p1 == P1_PRF_KEY_RECOVERY) {
        memmove(ctx->displayHeader, "Recover credentials", 20);
    } else if (p1 == P1_PRF_KEY) {
        memmove(ctx->displayHeader, "Decrypt", 8);
    }

    uiExportPrivateKey(flags);
}
