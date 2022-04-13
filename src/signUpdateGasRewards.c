#include <os.h>

#include "sign.h"
#include "util.h"

static signUpdateGasRewardsContext_t *ctx = &global.signUpdateGasRewardsContext;
static tx_state_t *tx_state = &global_tx_state;

UX_STEP_NOCB(
    ux_sign_gas_rewards_1_step,
    bnnn_paging,
    {.title = "Baker", .text = (char *) global.signUpdateGasRewardsContext.gasBaker});
UX_STEP_NOCB(
    ux_sign_gas_rewards_2_step,
    bnnn_paging,
    {.title = "Finalization proof", .text = (char *) global.signUpdateGasRewardsContext.gasFinalization});
UX_STEP_NOCB(
    ux_sign_gas_rewards_3_step,
    bnnn_paging,
    {.title = "Account creation", .text = (char *) global.signUpdateGasRewardsContext.gasAccountCreation});
UX_STEP_NOCB(
    ux_sign_gas_rewards_4_step,
    bnnn_paging,
    {.title = "Chain update", .text = (char *) global.signUpdateGasRewardsContext.gasChainUpdate});
UX_FLOW(
    ux_sign_gas_rewards,
    &ux_sign_flow_shared_review,
    &ux_sign_gas_rewards_1_step,
    &ux_sign_gas_rewards_2_step,
    &ux_sign_gas_rewards_3_step,
    &ux_sign_gas_rewards_4_step,
    &ux_sign_flow_shared_sign,
    &ux_sign_flow_shared_decline);

void handleSignUpdateGasRewards(uint8_t *cdata, volatile unsigned int *flags) {
    int bytesRead = parseKeyDerivationPath(cdata);
    cdata += bytesRead;

    cx_sha256_init(&tx_state->hash);
    cdata += hashUpdateHeaderAndType(cdata, UPDATE_TYPE_GAS_REWARDS);

    // Baker GAS bytes
    uint32_t gasBaker = U4BE(cdata, 0);
    fractionToText(gasBaker, ctx->gasBaker, sizeof(ctx->gasBaker));
    cx_hash((cx_hash_t *) &tx_state->hash, 0, cdata, 4, NULL, 0);
    cdata += 4;

    // Finalization proof GAS bytes
    uint32_t gasFinalizationProof = U4BE(cdata, 0);
    fractionToText(gasFinalizationProof, ctx->gasFinalization, sizeof(ctx->gasFinalization));
    cx_hash((cx_hash_t *) &tx_state->hash, 0, cdata, 4, NULL, 0);
    cdata += 4;

    // Account creation GAS bytes
    uint32_t gasAccountCreation = U4BE(cdata, 0);
    fractionToText(gasAccountCreation, ctx->gasAccountCreation, sizeof(ctx->gasAccountCreation));
    cx_hash((cx_hash_t *) &tx_state->hash, 0, cdata, 4, NULL, 0);
    cdata += 4;

    // Chain update GAS bytes
    uint32_t gasChainUpdate = U4BE(cdata, 0);
    fractionToText(gasChainUpdate, ctx->gasChainUpdate, sizeof(ctx->gasChainUpdate));
    cx_hash((cx_hash_t *) &tx_state->hash, 0, cdata, 4, NULL, 0);

    ux_flow_init(0, ux_sign_gas_rewards, NULL);
    *flags |= IO_ASYNCH_REPLY;
}
