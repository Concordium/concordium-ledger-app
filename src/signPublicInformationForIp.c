#include <os.h>

#include "responseCodes.h"
#include "sign.h"
#include "util.h"

static signPublicInformationForIp_t *ctx = &global.signPublicInformationForIp;
static tx_state_t *tx_state = &global_tx_state;

UX_STEP_CB(ux_sign_public_info_review, nn, sendSuccessNoIdle(), {"Review identity", "provider info"});
UX_FLOW(ux_review_public_info_for_ip, &ux_sign_public_info_review);

UX_STEP_CB(
    ux_sign_public_info_for_i_public_key_0_step,
    bnnn_paging,
    sendSuccessNoIdle(),
    {.title = "Public key", .text = (char *) global.signPublicInformationForIp.publicKey});
UX_FLOW(ux_sign_public_info_for_i_public_key, &ux_sign_public_info_for_i_public_key_0_step);

UX_STEP_CB(
    ux_sign_public_info_for_ip_sign,
    pnn,
    buildAndSignTransactionHash(),
    {&C_icon_validate_14, "Sign identity", "provider info"});

UX_STEP_CB(
    ux_sign_public_info_for_ip_decline,
    pnn,
    sendUserRejection(),
    {&C_icon_crossmark, "Decline to", "sign info"});

UX_STEP_NOCB(
    ux_sign_public_info_for_ip_threshold_0_step,
    bn,
    {"Signature threshold", (char *) global.signPublicInformationForIp.threshold});
UX_FLOW(
    ux_sign_public_info_for_ip_threshold,
    &ux_sign_public_info_for_ip_threshold_0_step,
    &ux_sign_public_info_for_ip_sign,
    &ux_sign_public_info_for_ip_decline);

#define P1_INITIAL          0x00
#define P1_VERIFICATION_KEY 0x01
#define P1_THRESHOLD        0x02

void handleSignPublicInformationForIp(uint8_t *cdata, uint8_t p1, volatile unsigned int *flags, bool isInitialCall) {
    if (isInitialCall) {
        ctx->state = TX_PUBLIC_INFO_FOR_IP_INITIAL;
    }

    if (p1 == P1_INITIAL && ctx->state == TX_PUBLIC_INFO_FOR_IP_INITIAL) {
        cdata += parseKeyDerivationPath(cdata);
        cx_sha256_init(&tx_state->hash);

        // We do not display IdCredPub as it is infeasible for the user to verify its correctness,
        // and maliciously replacing this value cannot give an attacker control of an account.
        cx_hash((cx_hash_t *) &tx_state->hash, 0, cdata, 48, NULL, 0);
        cdata += 48;

        // We do not display CredId as it is infeasible for the user to verify its correctness,
        // and maliciously replacing this value cannot give an attacker control of an account.
        cx_hash((cx_hash_t *) &tx_state->hash, 0, cdata, 48, NULL, 0);
        cdata += 48;

        // Parse number of public-keys that will be received next.
        ctx->publicKeysLength = cdata[0];
        cx_hash((cx_hash_t *) &tx_state->hash, 0, cdata, 1, NULL, 0);

        ctx->state = TX_PUBLIC_INFO_FOR_IP_VERIFICATION_KEY;
        ux_flow_init(0, ux_review_public_info_for_ip, NULL);
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
        toPaginatedHex(publicKey, 32, ctx->publicKey, sizeof(ctx->publicKey));

        ctx->publicKeysLength -= 1;
        if (ctx->publicKeysLength == 0) {
            ctx->state = TX_PUBLIC_INFO_FOR_IP_THRESHOLD;
        }
        ux_flow_init(0, ux_sign_public_info_for_i_public_key, NULL);
        *flags |= IO_ASYNCH_REPLY;
    } else if (p1 == P1_THRESHOLD && ctx->state == TX_PUBLIC_INFO_FOR_IP_THRESHOLD) {
        // Read the threshold byte and parse it to display it.
        cx_hash((cx_hash_t *) &tx_state->hash, 0, cdata, 1, NULL, 0);
        bin2dec(ctx->threshold, sizeof(ctx->threshold), cdata[0]);

        ux_flow_init(0, ux_sign_public_info_for_ip_threshold, NULL);
        *flags |= IO_ASYNCH_REPLY;
    } else {
        THROW(ERROR_INVALID_STATE);
    }
}
