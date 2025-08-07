#include "globals.h"

static keyDerivationPath_t *keyPath = &path;
static exportPublicKeyContext_t *ctx = &global.exportPublicKeyContext;
static tx_state_t *tx_state = &global_tx_state;
static const uint32_t HARDENED_OFFSET = 0x80000000;

instructionContext global;

/**
 * Derive the public-key for the given path, and then write it to
 * the APDU buffer to be returned to the caller.
 */
void sendPublicKey(bool compare) {
    uint8_t publicKey[KEY_LENGTH];
    getPublicKey(publicKey);

    // tx is holding the offset in the buffer we have written to. It is a convention to call this tx
    // for Ledger apps.
    uint8_t tx = 0;

    // Write the public-key to the APDU buffer.
    for (uint8_t i = 0; i < sizeof(publicKey); i++) {
        G_io_apdu_buffer[i] = publicKey[i];
        tx++;
    }

    if (ctx->signPublicKey) {
        uint8_t signedPublicKey[64];
        sign(publicKey, signedPublicKey);
        if (sizeof(signedPublicKey) > sizeof(G_io_apdu_buffer) - tx) {
            THROW(ERROR_BUFFER_OVERFLOW);
        }
        memmove(G_io_apdu_buffer + tx, signedPublicKey, sizeof(signedPublicKey));
        tx += sizeof(signedPublicKey);
    }

    // Send back success response including the public-key (and signature, if wanted).
    if (compare) {
        // Show the public-key so that the user can verify the public-key.
        sendSuccessResultNoIdle(tx);
        toPaginatedHex(publicKey, sizeof(publicKey), ctx->publicKey, sizeof(ctx->publicKey));
        // Allow for receiving a new instruction even while comparing public keys.
        tx_state->currentInstruction = -1;
        uiComparePubkey();

    } else {
        sendSuccess(tx);
    }
}

void handleGetPublicKey(uint8_t *cdata,
                        uint8_t p1,
                        uint8_t p2,
                        uint8_t lc,
                        volatile unsigned int *flags) {
    parseKeyDerivationPath(cdata, lc);

    // If P2 == 0x01, then the public-key is signed by its corresponding private key, and
    // appended to the returned public-key. This is used when it is needed to provide
    // proof of the knowledge of the corresponding private key.
    ctx->signPublicKey = p2 == 0x01;

    // If P1 == 0x01, then we skip displaying the key being exported. This is used when it
    // it is not important for the user to validate the key.
    if (p1 == 0x01) {
        sendPublicKey(false);
    } else {
        // If the key path is of length 5, then it is a request for a governance key.
        // Also it has to be in the governance subtree, which starts with 1.
        if (keyPath->pathLength == 5 && keyPath->rawKeyDerivationPath[0] == 1105) {
            if (keyPath->rawKeyDerivationPath[2] != 1) {
                THROW(ERROR_INVALID_PATH);
            }

            uint32_t purpose = keyPath->rawKeyDerivationPath[3];
            if (sizeof(ctx->display) < 13) {
                THROW(ERROR_BUFFER_OVERFLOW);
            }

            switch (purpose) {
                case 0:
                    memmove(ctx->display, "Gov. root", 10);
                    break;
                case 1:
                    memmove(ctx->display, "Gov. level 1", 13);
                    break;
                case 2:
                    memmove(ctx->display, "Gov. level 2", 13);
                    break;
                default:
                    THROW(ERROR_INVALID_PATH);
            }
        } else {
            if (keyPath->rawKeyDerivationPath[0] == 44 ||
                keyPath->rawKeyDerivationPath[0] == (44 | HARDENED_OFFSET)) {
                uint32_t identityProviderIndex = keyPath->rawKeyDerivationPath[2];
                uint32_t identityIndex = keyPath->rawKeyDerivationPath[3];
                uint32_t accountIndex = keyPath->rawKeyDerivationPath[5];
                getIdentityAccountDisplayNewPath(ctx->display,
                                                 sizeof(ctx->display),
                                                 identityProviderIndex,
                                                 identityIndex,
                                                 accountIndex);
            } else {
                uint32_t identityIndex = keyPath->rawKeyDerivationPath[4];
                uint32_t accountIndex = keyPath->rawKeyDerivationPath[6];
                getIdentityAccountDisplay(ctx->display,
                                          sizeof(ctx->display),
                                          identityIndex,
                                          accountIndex);
            }
        }
        uiGeneratePubkey(flags);
    }
}
