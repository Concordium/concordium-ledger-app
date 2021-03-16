#include <os.h>
#include <os_io_seproxyhal.h>
#include "cx.h"
#include <stdint.h>
#include "menu.h"
#include "util.h"
#include <string.h>
#include "sign.h"
#include "glyphs.h"

static signUpdateBakerStakeThresholdContext_t *ctx = &global.signUpdateBakerStakeThreshold;
static tx_state_t *tx_state = &global_tx_state;

UX_STEP_NOCB(
    ux_sign_baker_stake_threshold_0_step,
    nn,
    {
      "Review",
      "transaction"
    });
UX_STEP_NOCB(
    ux_sign_baker_stake_threshold_1_step,
    bn_paging,
    {
      .title = "Stake threshold",
      .text = (char *) global.signUpdateBakerStakeThreshold.stakeThreshold
    });
UX_STEP_CB(
    ux_sign_baker_stake_threshold_2_step,
    pnn,
    buildAndSignTransactionHash(),
    {
      &C_icon_validate_14,
      "Sign",
      "transaction"
    });
UX_STEP_CB(
    ux_sign_baker_stake_threshold_3_step,
    pnn,
    declineToSignTransaction(),
    {
      &C_icon_crossmark,
      "Decline to",
      "sign transaction"
    });
UX_FLOW(ux_sign_baker_stake_threshold,
    &ux_sign_baker_stake_threshold_0_step,
    &ux_sign_baker_stake_threshold_1_step,
    &ux_sign_baker_stake_threshold_2_step,
    &ux_sign_baker_stake_threshold_3_step
);

void handleSignUpdateBakerStakeThreshold(uint8_t *dataBuffer, volatile unsigned int *flags) {
    int bytesRead = parseKeyDerivationPath(dataBuffer);
    dataBuffer += bytesRead;

    cx_sha256_init(&tx_state->hash);
    cx_hash((cx_hash_t *) &tx_state->hash, 0, dataBuffer, UPDATE_HEADER_LENGTH, NULL, 0);
    dataBuffer += UPDATE_HEADER_LENGTH;
    uint8_t updateType = dataBuffer[0];
    if (updateType != 9) {
        THROW(SW_INVALID_TRANSACTION);
    }
    cx_hash((cx_hash_t *) &tx_state->hash, 0, dataBuffer, 1, NULL, 0);
    dataBuffer += 1;

    uint64_t bakerThresholdAmount = U8BE(dataBuffer, 0);
    bin2dec(ctx->stakeThreshold, bakerThresholdAmount);
    cx_hash((cx_hash_t *) &tx_state->hash, 0, dataBuffer, 8, NULL, 0);
    dataBuffer += 8;

    ux_flow_init(0, ux_sign_baker_stake_threshold, NULL);
    *flags |= IO_ASYNCH_REPLY;
}
