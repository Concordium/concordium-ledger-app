#include <os.h>
#include "util.h"
#include "sign.h"
#include "responseCodes.h"

static signPublicInformationForIp_t *ctx = &global.signPublicInformationForIp;
static tx_state_t *tx_state = &global_tx_state;

void loadSigningUx();

UX_STEP_CB(
    ux_sign_public_info_for_ip_1_step,
    bnnn_paging,
    sendSuccessNoIdle(),
    {
      .title = "CredId",
      .text = (char *) global.signPublicInformationForIp.credId
    });
UX_FLOW(ux_sign_public_info_for_ip,
    &ux_sign_flow_shared_review,
    &ux_sign_public_info_for_ip_1_step
);

UX_STEP_CB(
    ux_sign_public_info_for_i_public_key_0_step,
    bnnn_paging,
    sendSuccessNoIdle(),
    {
      .title = "Public key",
      .text = (char *) global.signPublicInformationForIp.publicKey
    });
UX_FLOW(ux_sign_public_info_for_i_public_key,
    &ux_sign_public_info_for_i_public_key_0_step
);

UX_STEP_NOCB(
    ux_sign_public_info_for_ip_threshold_0_step,
    bn,
    {
      "Sig Threshold",
      (char *) global.signPublicInformationForIp.threshold
    });
UX_FLOW(ux_sign_public_info_for_ip_threshold,
    &ux_sign_public_info_for_ip_threshold_0_step,
    &ux_sign_flow_shared_sign,
    &ux_sign_flow_shared_decline
);

#define P1_INITIAL              0x00
#define P1_VERIFICATION_KEY     0x01
#define P1_THRESHOLD            0x02

void handleSignPublicInformationForIp(uint8_t *cdata, uint8_t p1, volatile unsigned int *flags, bool isInitialCall) {
    if (isInitialCall) {
        ctx->state = TX_PUBLIC_INFO_FOR_IP_INITIAL;
    }

    if (p1 == P1_INITIAL && ctx->state == TX_PUBLIC_INFO_FOR_IP_INITIAL) {
        cdata += parseKeyDerivationPath(cdata);
        cx_sha256_init(&tx_state->hash);

        // Hash id_cred_pub.
        // We do not display the id_cred_pub, because it is infeasible for the user to verify the value, and replacing this value cannot give an attacker control of the account.
        cx_hash((cx_hash_t *) &tx_state->hash, 0, cdata, 48, NULL, 0);
        cdata += 48;

        // Parse cred_id so it can be displayed.
        uint8_t credId[48];
        memmove(credId, cdata, 48);
        cx_hash((cx_hash_t *) &tx_state->hash, 0, credId, 48, NULL, 0);
        cdata += 48;
        toHex(credId, 48, ctx->credId);

        // Parse number of public-keys that will be received next.
        ctx->publicKeysLength = cdata[0];
        cx_hash((cx_hash_t *) &tx_state->hash, 0, cdata, 1, NULL, 0);
        cdata += 1;

        ctx->state = TX_PUBLIC_INFO_FOR_IP_VERIFICATION_KEY;
        ux_flow_init(0, ux_sign_public_info_for_ip, NULL);
        *flags |= IO_ASYNCH_REPLY;
    } else if (p1 == P1_VERIFICATION_KEY && ctx->state == TX_PUBLIC_INFO_FOR_IP_VERIFICATION_KEY) {
        if (ctx->publicKeysLength <= 0) {
            THROW(ERROR_INVALID_STATE);
        }
        // Hash key index
        cx_hash((cx_hash_t *) &tx_state->hash, 0, cdata, 1, NULL, 0);
        cdata += 1;

        // Hash key type
        // We do not display the key type, as currently only ed_25519 is used.
        cx_hash((cx_hash_t *) &tx_state->hash, 0, cdata, 1, NULL, 0);
        cdata += 1;

        uint8_t publicKey[32];
        memmove(publicKey, cdata, 32);
        cx_hash((cx_hash_t *) &tx_state->hash, 0, publicKey, 32, NULL, 0);
        cdata += 32;
        toHex(publicKey, 32, ctx->publicKey);

        ctx->publicKeysLength -= 1;
        if (ctx->publicKeysLength == 0) {
            ctx->state = TX_PUBLIC_INFO_FOR_IP_THRESHOLD;
        }
        ux_flow_init(0, ux_sign_public_info_for_i_public_key, NULL);
        *flags |= IO_ASYNCH_REPLY;
    } else if (p1 == P1_THRESHOLD && ctx->state == TX_PUBLIC_INFO_FOR_IP_THRESHOLD) {
        // Read the threshold byte and parse it to display it.
        cx_hash((cx_hash_t *) &tx_state->hash, 0, cdata, 1, NULL, 0);
        bin2dec(ctx->threshold, cdata[0]);
        cdata += 1;

        ux_flow_init(0, ux_sign_public_info_for_ip_threshold, NULL);
        *flags |= IO_ASYNCH_REPLY;
    } else {
        THROW(ERROR_INVALID_STATE);
    }
}
