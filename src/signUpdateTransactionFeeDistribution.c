#include <os.h>

#include "responseCodes.h"
#include "sign.h"
#include "util.h"

static signTransactionDistributionFeeContext_t *ctx = &global.signTransactionDistributionFeeContext;
static tx_state_t *tx_state = &global_tx_state;

UX_STEP_NOCB(
    ux_sign_transaction_dist_1_step,
    bnnn_paging,
    {.title = "Baker fee", .text = (char *) global.signTransactionDistributionFeeContext.baker});
UX_STEP_NOCB(
    ux_sign_transaction_dist_2_step,
    bnnn_paging,
    {.title = "GAS account fee", .text = (char *) global.signTransactionDistributionFeeContext.gasAccount});
UX_FLOW(
    ux_sign_transaction_dist,
    &ux_sign_flow_shared_review,
    &ux_sign_transaction_dist_1_step,
    &ux_sign_transaction_dist_2_step,
    &ux_sign_flow_shared_sign,
    &ux_sign_flow_shared_decline);

void handleSignUpdateTransactionFeeDistribution(uint8_t *cdata, volatile unsigned int *flags) {
    cdata += parseKeyDerivationPath(cdata);

    cx_sha256_init(&tx_state->hash);
    cdata += hashUpdateHeaderAndType(cdata, UPDATE_TYPE_TRANSACTION_FEE_DISTRIBUTION);

    // Baker fee is first 4 bytes
    uint32_t bakerFee = U4BE(cdata, 0);
    fractionToPercentageDisplay(ctx->baker, sizeof(ctx->baker), bakerFee);
    cx_hash((cx_hash_t *) &tx_state->hash, 0, cdata, 4, NULL, 0);
    cdata += 4;

    // Gas account fee is the next 4 bytes
    uint32_t gasAccountFee = U4BE(cdata, 0);
    fractionToPercentageDisplay(ctx->gasAccount, sizeof(ctx->gasAccount), gasAccountFee);
    cx_hash((cx_hash_t *) &tx_state->hash, 0, cdata, 4, NULL, 0);

    ux_flow_init(0, ux_sign_transaction_dist, NULL);
    *flags |= IO_ASYNCH_REPLY;
}
