#include <os.h>
#include "util.h"
#include "sign.h"

static signUpdateMintDistribution_t *ctx = &global.signUpdateMintDistribution;
static tx_state_t *tx_state = &global_tx_state;

UX_STEP_NOCB(
    ux_sign_mint_rate_1_step,
    bnnn_paging,
    {
      .title = "Mint rate",
      .text = (char *) global.signUpdateMintDistribution.mintRate
    });
UX_STEP_NOCB(
    ux_sign_mint_rate_2_step,
    bnnn_paging,
    {
      .title = "Baker reward",
      .text = (char *) global.signUpdateMintDistribution.bakerReward
    });
UX_STEP_NOCB(
    ux_sign_mint_rate_3_step,
    bnnn_paging,
    {
      .title = "Finalization reward",
      .text = (char *) global.signUpdateMintDistribution.finalizationReward
    });
UX_FLOW(ux_sign_mint_rate,
    &ux_sign_flow_shared_review,
    &ux_sign_mint_rate_1_step,
    &ux_sign_mint_rate_2_step,
    &ux_sign_mint_rate_3_step,
    &ux_sign_flow_shared_sign,
    &ux_sign_flow_shared_decline
);

void handleSignUpdateMintDistribution(uint8_t *cdata, volatile unsigned int *flags) {
    int bytesRead = parseKeyDerivationPath(cdata);
    cdata += bytesRead;

    cx_sha256_init(&tx_state->hash);
    cdata += hashUpdateHeaderAndType(cdata, UPDATE_TYPE_MINT_DISTRIBUTION);

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

    // Baker reward
    uint8_t fraction[8] = "/100000";
    uint32_t bakerReward = U4BE(cdata, 0);
    int bakerRewardLength = numberToText(ctx->bakerReward, sizeof(ctx->bakerReward), bakerReward);
    cx_hash((cx_hash_t *) &tx_state->hash, 0, cdata, 4, NULL, 0);
    cdata += 4;
    memmove(ctx->bakerReward + bakerRewardLength, fraction, 8);

    // Finalization reward
    uint32_t finalizationReward = U4BE(cdata, 0);
    int finalizationRewardLength = numberToText(ctx->finalizationReward, sizeof(ctx->finalizationReward), finalizationReward);
    cx_hash((cx_hash_t *) &tx_state->hash, 0, cdata, 4, NULL, 0);
    memmove(ctx->finalizationReward + finalizationRewardLength, fraction, 8);

    ux_flow_init(0, ux_sign_mint_rate, NULL);
    *flags |= IO_ASYNCH_REPLY;
}
