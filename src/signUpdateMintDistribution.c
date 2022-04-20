#include <os.h>

#include "responseCodes.h"
#include "sign.h"
#include "util.h"

static signUpdateMintDistribution_t *ctx = &global.signUpdateMintDistribution;
static tx_state_t *tx_state = &global_tx_state;

UX_STEP_NOCB(
    ux_sign_mint_rate_1_step,
    bnnn_paging,
    {.title = "Baker reward", .text = (char *) global.signUpdateMintDistribution.bakerReward});
UX_STEP_NOCB(
    ux_sign_mint_rate_2_step,
    bnnn_paging,
    {.title = "Finalization reward", .text = (char *) global.signUpdateMintDistribution.finalizationReward});
UX_FLOW(
    ux_sign_mint_rate,
    &ux_sign_flow_shared_review,
    &ux_sign_mint_rate_1_step,
    &ux_sign_mint_rate_2_step,
    &ux_sign_flow_shared_sign,
    &ux_sign_flow_shared_decline);

#define P2_V1 0x01  // Does not include the mint rate

void handleSignUpdateMintDistribution(uint8_t *cdata, uint8_t p2, volatile unsigned int *flags) {
    if (p2 != P2_V1) {
        THROW(ERROR_INVALID_PARAM);
    }

    cdata += parseKeyDerivationPath(cdata);

    cx_sha256_init(&tx_state->hash);

    cdata += hashUpdateHeaderAndType(cdata, UPDATE_TYPE_MINT_DISTRIBUTION_V1);

    // Baker reward
    uint32_t bakerReward = U4BE(cdata, 0);
    fractionToText(bakerReward, ctx->bakerReward, sizeof(ctx->bakerReward));
    cx_hash((cx_hash_t *) &tx_state->hash, 0, cdata, 4, NULL, 0);
    cdata += 4;

    // Finalization reward
    uint32_t finalizationReward = U4BE(cdata, 0);
    fractionToText(finalizationReward, ctx->finalizationReward, sizeof(ctx->finalizationReward));
    cx_hash((cx_hash_t *) &tx_state->hash, 0, cdata, 4, NULL, 0);

    ux_flow_init(0, ux_sign_mint_rate, NULL);
    *flags |= IO_ASYNCH_REPLY;
}
