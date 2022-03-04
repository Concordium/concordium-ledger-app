#include <os.h>

#include "sign.h"
#include "util.h"

static signUpdateTimeParametersContext_t *ctx = &global.signTimeParameters;
static tx_state_t *tx_state = &global_tx_state;

UX_STEP_NOCB(
    ux_sign_time_parameters_1_step,
    bnnn_paging,
    {.title = "Reward Period Length", .text = (char *) global.signTimeParameters.rewardPeriodLength});
UX_STEP_NOCB(
    ux_sign_time_parameters_2_step,
    bnnn_paging,
    {.title = "Mint rate", .text = (char *) global.signTimeParameters.mintRate});
UX_FLOW(
    ux_sign_time_parameters,
    &ux_sign_flow_shared_review,
    &ux_sign_time_parameters_1_step,
    &ux_sign_time_parameters_2_step,
    &ux_sign_flow_shared_sign,
    &ux_sign_flow_shared_decline);

void handleSignUpdateTimeParameters(uint8_t *cdata, volatile unsigned int *flags) {
    int bytesRead = parseKeyDerivationPath(cdata);
    cdata += bytesRead;

    cx_sha256_init(&tx_state->hash);
    cdata += hashUpdateHeaderAndType(cdata, UPDATE_TYPE_TIME_PARAMETERS);

    //RewardPeriodLength is a 64-bit number
    uint64_t rewardPeriodLength = U8BE(cdata, 0);
    cx_hash((cx_hash_t *) &tx_state->hash, 0, cdata, 8, NULL, 0);
    cdata += 8;
    numberToText(ctx->rewardPeriodLength, sizeof(ctx->rewardPeriodLength), rewardPeriodLength);

    // Mint rate consists of 4 bytes of mantissa, and a 1 byte exponent.
    uint32_t mintRateMantissa = U4BE(cdata, 0);
    uint8_t mintRateExponent = cdata[4];
    cx_hash((cx_hash_t *) &tx_state->hash, 0, cdata, 5, NULL, 0);
    cdata += 5;

    // Build display of the mint rate as 'mintRateMantissa*10^(-mintRateExponent)'
    int offset = numberToText(ctx->mintRate, sizeof(ctx->mintRate), mintRateMantissa);
    uint8_t multiplication[6] = "*10^(-";
    memmove(ctx->mintRate + offset, multiplication, 6);
    offset += 6;
    offset += numberToText(ctx->mintRate + offset, sizeof(ctx->mintRate) - offset, mintRateExponent);
    memmove(ctx->mintRate + offset, ")", 2);

    ux_flow_init(0, ux_sign_time_parameters, NULL);
    *flags |= IO_ASYNCH_REPLY;
}
