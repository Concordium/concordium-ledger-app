#include <os.h>

#include "sign.h"
#include "util.h"

static signUpdateGasRewardsContext_t *ctx = &global.signUpdateGasRewardsContext;
static tx_state_t *tx_state = &global_tx_state;

UX_STEP_NOCB(
    ux_sign_gas_rewards_type_step,
    bn,
    {"Update type", (char *) global.signUpdateGasRewardsContext.updateTypeText});
UX_STEP_NOCB(
    ux_sign_gas_rewards_1_step,
    bnnn_paging,
    {.title = "Baker", .text = (char *) global.signUpdateGasRewardsContext.gasBaker});
UX_STEP_NOCB(
    ux_sign_gas_rewards_2_step,
    bnnn_paging,
    {.title = "Account creation", .text = (char *) global.signUpdateGasRewardsContext.gasAccountCreation});
UX_STEP_NOCB(
    ux_sign_gas_rewards_3_step,
    bnnn_paging,
    {.title = "Chain update", .text = (char *) global.signUpdateGasRewardsContext.gasChainUpdate});
UX_FLOW(
    ux_sign_gas_rewards,
    &ux_sign_flow_shared_review,
    &ux_sign_gas_rewards_type_step,
    &ux_sign_gas_rewards_1_step,
    &ux_sign_gas_rewards_2_step,
    &ux_sign_gas_rewards_3_step,
    &ux_sign_flow_shared_sign,
    &ux_sign_flow_shared_decline);

void handleSignUpdateGasRewards(uint8_t *cdata, volatile unsigned int *flags) {
    cdata += parseKeyDerivationPath(cdata);
    cx_sha256_init(&tx_state->hash);
    cdata += hashUpdateHeaderAndType(cdata, UPDATE_TYPE_GAS_REWARDS_CPV2);

    // Set update type text
    strncpy(ctx->updateTypeText, getUpdateTypeText(UPDATE_TYPE_GAS_REWARDS_CPV2), sizeof(ctx->updateTypeText));
    ctx->updateTypeText[sizeof(ctx->updateTypeText) - 1] = '\0';

    uint32_t gasBaker = U4BE(cdata, 0);
    uint32_t gasAccountCreation = U4BE(cdata, 4);
    uint32_t gasChainUpdate = U4BE(cdata, 8);

    updateHash((cx_hash_t *) &tx_state->hash, cdata, 12);

    fractionToPercentageDisplay(ctx->gasBaker, sizeof(ctx->gasBaker), gasBaker);    
    fractionToPercentageDisplay(ctx->gasAccountCreation, sizeof(ctx->gasAccountCreation), gasAccountCreation);
    fractionToPercentageDisplay(ctx->gasChainUpdate, sizeof(ctx->gasChainUpdate), gasChainUpdate);

    ux_flow_init(0, ux_sign_gas_rewards, NULL);
    *flags |= IO_ASYNCH_REPLY;
}
