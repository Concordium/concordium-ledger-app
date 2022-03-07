#include <os.h>

#include "sign.h"
#include "util.h"

static signUpdateCooldownParametersContext_t *ctx = &global.signCooldownParameters;
static tx_state_t *tx_state = &global_tx_state;

UX_STEP_NOCB(
    ux_sign_cooldown_parameters_1_step,
    bnnn_paging,
    {.title = "Delegator cooldown", .text = (char *) global.signCooldownParameters.delegatorCooldown});
UX_STEP_NOCB(
    ux_sign_cooldown_parameters_2_step,
    bnnn_paging,
    {.title = "Pool owner cooldown", .text = (char *) global.signCooldownParameters.poolOwnerCooldown});
UX_FLOW(
    ux_sign_cooldown_parameters,
    &ux_sign_flow_shared_review,
    &ux_sign_cooldown_parameters_1_step,
    &ux_sign_cooldown_parameters_2_step,
    &ux_sign_flow_shared_sign,
    &ux_sign_flow_shared_decline);

void handleSignUpdateCooldownParameters(uint8_t *cdata, volatile unsigned int *flags) {
    int bytesRead = parseKeyDerivationPath(cdata);
    cdata += bytesRead;

    cx_sha256_init(&tx_state->hash);
    cdata += hashUpdateHeaderAndType(cdata, UPDATE_TYPE_COOLDOWN_PARAMETERS);

    // RewardPeriodLength is a 64-bit number
    uint64_t delegatorCooldown = U8BE(cdata, 0);
    cx_hash((cx_hash_t *) &tx_state->hash, 0, cdata, 8, NULL, 0);
    cdata += 8;
    numberToText(ctx->delegatorCooldown, sizeof(ctx->delegatorCooldown), delegatorCooldown);

    // RewardPeriodLength is a 64-bit number
    uint64_t poolOwnerCooldown = U8BE(cdata, 0);
    cx_hash((cx_hash_t *) &tx_state->hash, 0, cdata, 8, NULL, 0);
    numberToText(ctx->poolOwnerCooldown, sizeof(ctx->poolOwnerCooldown), poolOwnerCooldown);

    ux_flow_init(0, ux_sign_cooldown_parameters, NULL);
    *flags |= IO_ASYNCH_REPLY;
}
