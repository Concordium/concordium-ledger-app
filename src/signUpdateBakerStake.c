#include <os.h>
#include <os_io_seproxyhal.h>
#include "cx.h"
#include <stdint.h>
#include "menu.h"
#include "util.h"
#include <string.h>
#include "base58check.h"
#include <stdio.h>
#include "sign.h"

static signUpdateBakerStakeContext_t *ctx = &global.signUpdateBakerStake;
static tx_state_t *tx_state = &global_tx_state;

UX_STEP_NOCB(
    ux_sign_update_baker_stake_0_step,
    nn,
    {
      "Review",
      "transaction"
    });
UX_STEP_NOCB(
    ux_sign_update_baker_stake_1_step,
    bn_paging,
    {
      .title = "Update stake to",
      .text = (char *) global.signUpdateBakerStake.amount
    });
UX_STEP_CB(
    ux_sign_update_baker_stake_2_step,
    pnn,
    buildAndSignTransactionHash(),
    {
      &C_icon_validate_14,
      "Sign",
      "transaction"
    });
UX_STEP_CB(
    ux_sign_update_baker_stake_3_step,
    pnn,
    declineToSignTransaction(),
    {
      &C_icon_crossmark,
      "Decline to",
      "sign transaction"
    });
UX_FLOW(ux_sign_update_baker_stake,
    &ux_sign_update_baker_stake_0_step,
    &ux_sign_update_baker_stake_1_step,
    &ux_sign_update_baker_stake_2_step,
    &ux_sign_update_baker_stake_3_step
);

void handleSignUpdateBakerStake(uint8_t *dataBuffer, volatile unsigned int *flags) {
    int bytesRead = parseKeyDerivationPath(dataBuffer);
    dataBuffer += bytesRead;

    cx_sha256_init(&tx_state->hash);
    cx_hash((cx_hash_t *) &tx_state->hash, 0, dataBuffer, ACCOUNT_TRANSACTION_HEADER_LENGTH + 1, NULL, 0);
    uint8_t transactionKind = dataBuffer[ACCOUNT_TRANSACTION_HEADER_LENGTH];
    if (transactionKind != UPDATE_BAKER_STAKE) {
        THROW(SW_INVALID_TRANSACTION);
    }
    dataBuffer += (ACCOUNT_TRANSACTION_HEADER_LENGTH + 1);

    // Parse the amount to update the stake to, so it can be displayed for verification.
    uint64_t amount = U8BE(dataBuffer, 0);
    bin2dec(ctx->amount, amount);
    cx_hash((cx_hash_t *) &tx_state->hash, 0, dataBuffer, 8, NULL, 0);
    dataBuffer += 8;

    ux_flow_init(0, ux_sign_update_baker_stake, NULL);
    *flags |= IO_ASYNCH_REPLY;
}
