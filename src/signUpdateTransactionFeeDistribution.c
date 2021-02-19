#include <os.h>
#include <os_io_seproxyhal.h>
#include "cx.h"
#include <stdint.h>
#include "menu.h"
#include "util.h"
#include <string.h>
#include "sign.h"

static signTransactionDistributionFeeContext_t *ctx = &global.signTransactionDistributionFeeContext;
static tx_state_t *tx_state = &global_tx_state;

UX_STEP_NOCB(
    ux_sign_transaction_dist_0_step,
    nn,
    {
      "Review",
      "transaction"
    });
UX_STEP_NOCB(
    ux_sign_transaction_dist_1_step,
    bn_paging,
    {
      .title = "Baker fee",
      .text = (char *) global.signTransactionDistributionFeeContext.baker
    });
UX_STEP_NOCB(
    ux_sign_transaction_dist_2_step,
    bn_paging,
    {
      .title = "GAS account fee",
      .text = (char *) global.signTransactionDistributionFeeContext.gasAccount
    });
UX_STEP_CB(
    ux_sign_transaction_dist_3_step,
    pnn,
    buildAndSignTransactionHash(),
    {
      &C_icon_validate_14,
      "Sign",
      "transaction"
    });
UX_STEP_CB(
    ux_sign_transaction_dist_4_step,
    pnn,
    declineToSignTransaction(),
    {
      &C_icon_crossmark,
      "Decline to",
      "sign transaction"
    });
UX_FLOW(ux_sign_transaction_dist,
    &ux_sign_transaction_dist_0_step,
    &ux_sign_transaction_dist_1_step,
    &ux_sign_transaction_dist_2_step,
    &ux_sign_transaction_dist_3_step,
    &ux_sign_transaction_dist_4_step
);

void handleSignUpdateTransactionFeeDistribution(uint8_t *dataBuffer, volatile unsigned int *flags) {
    int bytesRead = parseKeyDerivationPath(dataBuffer);
    dataBuffer += bytesRead;

    cx_sha256_init(&tx_state->hash);

    // Add UpdateHeader to hash.
    cx_hash((cx_hash_t *) &tx_state->hash, 0, dataBuffer, UPDATE_HEADER_LENGTH, NULL, 0);
    dataBuffer += UPDATE_HEADER_LENGTH;

    // All update transactions are pre-pended by their type.
    uint8_t updateType = dataBuffer[0];
    cx_hash((cx_hash_t *) &tx_state->hash, 0, dataBuffer, 1, NULL, 0);
    dataBuffer += 1;

    if (updateType != 7) {
        // Received an incorrect update type byte.
        THROW(0x6B01);
    }

    // Baker fee is first 4 bytes
    uint32_t bakerFee = U4BE(dataBuffer, 0);
    int bakerFeeLength = bin2dec(ctx->baker, bakerFee);
    cx_hash((cx_hash_t *) &tx_state->hash, 0, dataBuffer, 4, NULL, 0);
    dataBuffer += 4;
    uint8_t fraction[10] = "/100000";
    os_memmove(ctx->baker + bakerFeeLength, fraction, 10);

    // Gas account fee is the next 4 bytes
    uint32_t gasAccountFee = U4BE(dataBuffer, 0);
    int gasAccountFeeLength = bin2dec(ctx->gasAccount, gasAccountFee);
    cx_hash((cx_hash_t *) &tx_state->hash, 0, dataBuffer, 4, NULL, 0);
    dataBuffer += 4;
    os_memmove(ctx->gasAccount + gasAccountFeeLength, fraction, 9);

    ux_flow_init(0, ux_sign_transaction_dist, NULL);
    *flags |= IO_ASYNCH_REPLY;
}
