#include <os.h>
#include "util.h"
#include "sign.h"

static signUpdateBakerStakeThresholdContext_t *ctx = &global.signUpdateBakerStakeThreshold;
static tx_state_t *tx_state = &global_tx_state;

UX_STEP_NOCB(
    ux_sign_baker_stake_threshold_1_step,
    bn_paging,
    {
      .title = "Stake threshold",
      .text = (char *) global.signUpdateBakerStakeThreshold.stakeThreshold
    });
UX_FLOW(ux_sign_baker_stake_threshold,
    &ux_sign_flow_shared_review,
    &ux_sign_baker_stake_threshold_1_step,
    &ux_sign_flow_shared_sign,
    &ux_sign_flow_shared_decline
);

void handleSignUpdateBakerStakeThreshold(uint8_t *cdata, volatile unsigned int *flags) {
    cdata += parseKeyDerivationPath(cdata);
    cx_sha256_init(&tx_state->hash);
    cdata += hashUpdateHeaderAndType(cdata, UPDATE_TYPE_BAKER_STAKE_THRESHOLD);

    uint64_t bakerThresholdAmount = U8BE(cdata, 0);
    bin2dec(ctx->stakeThreshold, bakerThresholdAmount);
    cx_hash((cx_hash_t *) &tx_state->hash, 0, cdata, 8, NULL, 0);
    cdata += 8;

    ux_flow_init(0, ux_sign_baker_stake_threshold, NULL);
    *flags |= IO_ASYNCH_REPLY;
}
