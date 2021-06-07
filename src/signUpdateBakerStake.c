#include <os.h>
#include "util.h"
#include "accountSenderView.h"
#include "sign.h"

static signUpdateBakerStakeContext_t *ctx = &global.signUpdateBakerStake;
static tx_state_t *tx_state = &global_tx_state;

UX_STEP_NOCB(
    ux_sign_update_baker_stake_1_step,
    bn_paging,
    {
      .title = "Update stake to",
      .text = (char *) global.signUpdateBakerStake.amount
    });
UX_FLOW(ux_sign_update_baker_stake,
    &ux_sign_flow_shared_review,
    &ux_sign_flow_account_sender_view,
    &ux_sign_update_baker_stake_1_step,
    &ux_sign_flow_shared_sign,
    &ux_sign_flow_shared_decline
);

void handleSignUpdateBakerStake(uint8_t *cdata, volatile unsigned int *flags) {
    cdata += parseKeyDerivationPath(cdata);
    cx_sha256_init(&tx_state->hash);
    cdata += hashAccountTransactionHeaderAndKind(cdata, UPDATE_BAKER_STAKE);

    // Parse the amount to update the stake to, so it can be displayed for verification.
    uint64_t amount = U8BE(cdata, 0);
    amountToGtuDisplay(ctx->amount, amount);
    cx_hash((cx_hash_t *) &tx_state->hash, 0, cdata, 8, NULL, 0);
    cdata += 8;

    ux_flow_init(0, ux_sign_update_baker_stake, NULL);
    *flags |= IO_ASYNCH_REPLY;
}
