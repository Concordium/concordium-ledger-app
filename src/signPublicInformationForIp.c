#include "globals.h"

static signPublicInformationForIp_t *ctx = &global.signPublicInformationForIp;
static tx_state_t *tx_state = &global_tx_state;

#define P1_INITIAL          0x00
#define P1_VERIFICATION_KEY 0x01
#define P1_THRESHOLD        0x02

void handleSignPublicInformationForIp(uint8_t *cdata,
                                      uint8_t p1,
                                      volatile unsigned int *flags,
                                      bool isInitialCall) {
    if (isInitialCall) {
        ctx->state = TX_PUBLIC_INFO_FOR_IP_INITIAL;
    }

    if (p1 == P1_INITIAL && ctx->state == TX_PUBLIC_INFO_FOR_IP_INITIAL) {
        cdata += parseKeyDerivationPath(cdata);
        cx_sha256_init(&tx_state->hash);

        // We do not display IdCredPub as it is infeasible for the user to verify its correctness,
        // and maliciously replacing this value cannot give an attacker control of an account.
        updateHash((cx_hash_t *)&tx_state->hash, cdata, 48);
        cdata += 48;

        // We do not display CredId as it is infeasible for the user to verify its correctness,
        // and maliciously replacing this value cannot give an attacker control of an account.
        updateHash((cx_hash_t *)&tx_state->hash, cdata, 48);
        cdata += 48;

        // Parse number of public-keys that will be received next.
        ctx->publicKeysLength = cdata[0];
        updateHash((cx_hash_t *)&tx_state->hash, cdata, 1);

        ctx->showIntro = true;
        ctx->state = TX_PUBLIC_INFO_FOR_IP_VERIFICATION_KEY;
        sendSuccessNoIdle();
    } else if (p1 == P1_VERIFICATION_KEY && ctx->state == TX_PUBLIC_INFO_FOR_IP_VERIFICATION_KEY) {
        if (ctx->publicKeysLength <= 0) {
            THROW(ERROR_INVALID_STATE);
        }
        // Hash key index
        updateHash((cx_hash_t *)&tx_state->hash, cdata, 1);
        cdata += 1;

        // Hash key type
        // We do not display the key type, as currently only ed_25519 is used.
        updateHash((cx_hash_t *)&tx_state->hash, cdata, 1);
        cdata += 1;

        uint8_t publicKey[32];
        memmove(publicKey, cdata, 32);
        updateHash((cx_hash_t *)&tx_state->hash, publicKey, 32);
        toPaginatedHex(publicKey, 32, ctx->publicKey, sizeof(ctx->publicKey));

        ctx->publicKeysLength -= 1;
        if (ctx->publicKeysLength > 0) {
            if (ctx->showIntro) {
                // For the first key, we also display the initial view
                ctx->showIntro = false;
                uiReviewPublicInformationForIpDisplay();
            } else {
                uiSignPublicInformationForIpPublicKeyDisplay();
            }
            *flags |= IO_ASYNCH_REPLY;
        } else {
            ctx->state = TX_PUBLIC_INFO_FOR_IP_THRESHOLD;
            // We don't display the last public key here. It is displayed in the final flow.
            sendSuccessNoIdle();
        }
    } else if (p1 == P1_THRESHOLD && ctx->state == TX_PUBLIC_INFO_FOR_IP_THRESHOLD) {
        // Read the threshold byte and parse it to display it.
        updateHash((cx_hash_t *)&tx_state->hash, cdata, 1);
        bin2dec(ctx->threshold, sizeof(ctx->threshold), cdata[0]);

        if (ctx->showIntro) {
            // If the initial view has not been displayed yet, we display the entire flow
            uiSignPublicInformationForIpCompleteDisplay();
        } else {
            uiSignPublicInformationForIpFinalDisplay();
        }
        *flags |= IO_ASYNCH_REPLY;
    } else {
        THROW(ERROR_INVALID_STATE);
    }
}
