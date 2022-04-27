#include <os.h>

#include "responseCodes.h"
#include "sign.h"
#include "util.h"

static signUpdatePoolParametersContext_t *ctx = &global.signPoolParameters;
static tx_state_t *tx_state = &global_tx_state;

UX_STEP_NOCB(
    ux_sign_pool_parameters_finalization_reward_step,
    bn,
    {"P.D. finalize reward", (char *) global.signPoolParameters.passiveFinalizationRewardCommissionRate});
UX_STEP_NOCB(
    ux_sign_pool_parameters_baking_reward_step,
    bn,
    {"P.D. baking reward", (char *) global.signPoolParameters.passiveBakingRewardCommissionRate});
UX_STEP_CB(
    ux_sign_pool_parameters_transaction_fee_step,
    bn,
    sendSuccessNoIdle(),
    {"P.D. transaction fee", (char *) global.signPoolParameters.passiveTransactionFeeCommissionRate});

UX_STEP_NOCB(
    ux_sign_pool_parameters_finalization_reward_max_step,
    bn,
    {"Max finalize reward", (char *) global.signPoolParameters.finalizationRewardCommissionRateMax});
UX_STEP_NOCB(
    ux_sign_pool_parameters_baking_reward_max_step,
    bn,
    {"Max baking reward", (char *) global.signPoolParameters.bakingRewardCommissionRateMax});
UX_STEP_CB(
    ux_sign_pool_parameters_transaction_fee_max_step,
    bn,
    sendSuccessNoIdle(),
    {"Max transaction fee", (char *) global.signPoolParameters.transactionFeeCommissionRateMax});
UX_STEP_NOCB(
    ux_sign_pool_parameters_finalization_reward_min_step,
    bn,
    {"Min finalize reward", (char *) global.signPoolParameters.finalizationRewardCommissionRateMin});
UX_STEP_NOCB(
    ux_sign_pool_parameters_baking_reward_min_step,
    bn,
    {"Min baking reward", (char *) global.signPoolParameters.bakingRewardCommissionRateMin});
UX_STEP_NOCB(
    ux_sign_pool_parameters_transaction_fee_min_step,
    bn,
    {"Min transaction fee", (char *) global.signPoolParameters.transactionFeeCommissionRateMin});

UX_STEP_NOCB(
    ux_sign_pool_parameters_minimum_capital_step,
    bnnn_paging,
    {.title = "Min equity capital", .text = (char *) global.signPoolParameters.minimumEquityCapital});
UX_STEP_NOCB(
    ux_sign_pool_parameters_capital_bound_step,
    bn,
    {"Capital bound", (char *) global.signPoolParameters.capitalBound});
UX_STEP_NOCB(
    ux_sign_pool_parameters_leverage_bound_step,
    bnnn_paging,
    {.title = "Leverage bound", .text = (char *) global.signPoolParameters.leverageBound});

UX_FLOW(
    ux_sign_pool_parameters_initial,
    &ux_sign_flow_shared_review,
    &ux_sign_pool_parameters_finalization_reward_step,
    &ux_sign_pool_parameters_baking_reward_step,
    &ux_sign_pool_parameters_transaction_fee_step);
UX_FLOW(
    ux_sign_pool_parameters_commission_ranges,
    &ux_sign_pool_parameters_finalization_reward_min_step,
    &ux_sign_pool_parameters_finalization_reward_max_step,
    &ux_sign_pool_parameters_baking_reward_min_step,
    &ux_sign_pool_parameters_baking_reward_max_step,
    &ux_sign_pool_parameters_transaction_fee_min_step,
    &ux_sign_pool_parameters_transaction_fee_max_step);
UX_FLOW(
    ux_sign_pool_parameters_commission_final,
    &ux_sign_pool_parameters_minimum_capital_step,
    &ux_sign_pool_parameters_capital_bound_step,
    &ux_sign_pool_parameters_leverage_bound_step,
    &ux_sign_flow_shared_sign,
    &ux_sign_flow_shared_decline);

#define P1_INITIAL           0x00
#define P1_COMMISSION_RANGES 0x01
#define P1_EQUITY            0x02

/**
 * Helper method for parsing commission rates as they are all equal in structure.
 */
uint8_t parseCommissionRate(uint8_t *cdata, uint8_t *commissionRateDisplay, uint8_t sizeOfCommissionRateDisplay) {
    uint32_t rate = U4BE(cdata, 0);
    fractionToText(rate, commissionRateDisplay, sizeOfCommissionRateDisplay);
    cx_hash((cx_hash_t *) &tx_state->hash, 0, cdata, 4, NULL, 0);
    return 4;
}

void handleSignUpdatePoolParameters(uint8_t *cdata, uint8_t p1, volatile unsigned int *flags, bool isInitialCall) {
    if (isInitialCall) {
        ctx->state = TX_UPDATE_POOL_PARAMETERS_INITIAL;
    }

    if (p1 == P1_INITIAL && ctx->state == TX_UPDATE_POOL_PARAMETERS_INITIAL) {
        cdata += parseKeyDerivationPath(cdata);

        cx_sha256_init(&tx_state->hash);
        cdata += hashUpdateHeaderAndType(cdata, UPDATE_TYPE_POOL_PARAMETERS);

        cdata += parseCommissionRate(
            cdata,
            ctx->passiveFinalizationRewardCommissionRate,
            sizeof(ctx->passiveFinalizationRewardCommissionRate));
        cdata += parseCommissionRate(
            cdata,
            ctx->passiveBakingRewardCommissionRate,
            sizeof(ctx->passiveBakingRewardCommissionRate));
        parseCommissionRate(
            cdata,
            ctx->passiveTransactionFeeCommissionRate,
            sizeof(ctx->passiveTransactionFeeCommissionRate));

        ctx->state = TX_UPDATE_POOL_PARAMETERS_RANGES;

        ux_flow_init(0, ux_sign_pool_parameters_initial, NULL);
        *flags |= IO_ASYNCH_REPLY;
    } else if (p1 == P1_COMMISSION_RANGES && ctx->state == TX_UPDATE_POOL_PARAMETERS_RANGES) {
        cdata += parseCommissionRate(
            cdata,
            ctx->finalizationRewardCommissionRateMin,
            sizeof(ctx->finalizationRewardCommissionRateMin));
        cdata += parseCommissionRate(
            cdata,
            ctx->finalizationRewardCommissionRateMax,
            sizeof(ctx->finalizationRewardCommissionRateMax));

        cdata +=
            parseCommissionRate(cdata, ctx->bakingRewardCommissionRateMin, sizeof(ctx->bakingRewardCommissionRateMin));
        cdata +=
            parseCommissionRate(cdata, ctx->bakingRewardCommissionRateMax, sizeof(ctx->bakingRewardCommissionRateMax));

        cdata += parseCommissionRate(
            cdata,
            ctx->transactionFeeCommissionRateMin,
            sizeof(ctx->transactionFeeCommissionRateMin));
        parseCommissionRate(cdata, ctx->transactionFeeCommissionRateMax, sizeof(ctx->transactionFeeCommissionRateMax));

        ctx->state = TX_UPDATE_POOL_PARAMETERS_EQUITY;

        ux_flow_init(0, ux_sign_pool_parameters_commission_ranges, NULL);
        *flags |= IO_ASYNCH_REPLY;
    } else if (p1 == P1_EQUITY && ctx->state == TX_UPDATE_POOL_PARAMETERS_EQUITY) {
        uint64_t minimumEquityCapital = U8BE(cdata, 0);
        amountToGtuDisplay(ctx->minimumEquityCapital, sizeof(ctx->minimumEquityCapital), minimumEquityCapital);
        cx_hash((cx_hash_t *) &tx_state->hash, 0, cdata, 8, NULL, 0);
        cdata += 8;

        cdata += parseCommissionRate(cdata, ctx->capitalBound, sizeof(ctx->capitalBound));

        uint64_t leverageBoundNumerator = U8BE(cdata, 0);
        cx_hash((cx_hash_t *) &tx_state->hash, 0, cdata, 8, NULL, 0);
        cdata += 8;

        uint64_t leverageBoundDenominator = U8BE(cdata, 0);
        cx_hash((cx_hash_t *) &tx_state->hash, 0, cdata, 8, NULL, 0);

        int numLength = numberToText(ctx->leverageBound, sizeof(ctx->leverageBound), leverageBoundNumerator);
        memmove(ctx->leverageBound + numLength, "/", 1);
        numberToText(
            ctx->leverageBound + numLength + 1,
            sizeof(ctx->leverageBound) - (numLength + 1),
            leverageBoundDenominator);

        ux_flow_init(0, ux_sign_pool_parameters_commission_final, NULL);
        *flags |= IO_ASYNCH_REPLY;
    } else {
        THROW(ERROR_INVALID_STATE);
    }
}
