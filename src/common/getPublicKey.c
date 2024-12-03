#include "globals.h"
#include "menu.h"
#include "os.h"
#include "responseCodes.h"
#include "util.h"
#include "ux.h"

static keyDerivationPath_t *keyPath = &path;
static exportPublicKeyContext_t *ctx = &global.exportPublicKeyContext;
static tx_state_t *tx_state = &global_tx_state;

void sendPublicKey(bool compare);

UX_STEP_VALID(ux_decline_step, pb, sendUserRejection(), {&C_icon_crossmark, "Decline"});

// UI definitions for the approval of the generation of a public-key. This prompts the user to
// accept that a public-key will be generated and returned to the computer.
UX_STEP_VALID(ux_generate_public_flow_0_step,
              pnn,
              sendPublicKey(true),
              {&C_icon_validate_14, "Public key", (char *) global.exportPublicKeyContext.display});
UX_FLOW(ux_generate_public_flow, &ux_generate_public_flow_0_step, &ux_decline_step, FLOW_LOOP);

// UI definitions for comparison of public-key on the device
// with the public-key that the caller received.
UX_STEP_NOCB(ux_sign_compare_public_key_0_step,
             bnnn_paging,
             {.title = "Compare", .text = (char *) global.exportPublicKeyContext.publicKey});
UX_STEP_CB(ux_compare_accept_step, pb, ui_idle(), {&C_icon_validate_14, "Accept"});
UX_STEP_CB(ux_compare_decline_step, pb, ui_idle(), {&C_icon_crossmark, "Decline"});
UX_FLOW(ux_sign_compare_public_key,
        &ux_sign_compare_public_key_0_step,
        &ux_compare_accept_step,
        &ux_compare_decline_step);

/**
 * Derive the public-key for the given path, and then write it to
 * the APDU buffer to be returned to the caller.
 */
void sendPublicKey(bool compare) {
    uint8_t publicKey[32];
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
        ux_flow_init(0, ux_sign_compare_public_key, NULL);
    } else {
        sendSuccess(tx);
    }
}

void handleGetPublicKey(uint8_t *cdata, uint8_t p1, uint8_t p2, volatile unsigned int *flags) {
    parseKeyDerivationPath(cdata);

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
        if (keyPath->pathLength == 5) {
            if (keyPath->rawKeyDerivationPath[2] != 1) {
                THROW(ERROR_INVALID_PATH);
            }

            uint32_t purpose = keyPath->rawKeyDerivationPath[3];

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
            uint32_t identityIndex = keyPath->rawKeyDerivationPath[4];
            uint32_t accountIndex = keyPath->rawKeyDerivationPath[6];
            getIdentityAccountDisplay(ctx->display,
                                      sizeof(ctx->display),
                                      identityIndex,
                                      accountIndex);
        }

        // Display the UI for the public-key flow, where the user can validate that the
        // public-key being generated is the expected one.
        ux_flow_init(0, ux_generate_public_flow, NULL);

        // Tell the main process to wait for a button press.
        *flags |= IO_ASYNCH_REPLY;
    }
}
