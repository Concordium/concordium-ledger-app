#include <os.h>
#include <os_io_seproxyhal.h>
#include "cx.h"
#include <stdint.h>
#include "menu.h"
#include "util.h"
#include <string.h>
#include "sign.h"

static signUpdateGasRewardsContext_t *ctx = &global.signUpdateGasRewardsContext;
static tx_state_t *tx_state = &global_tx_state;

UX_STEP_NOCB(
    ux_sign_gas_rewards_0_step,
    nn,
    {
      "Review",
      "transaction"
    });
UX_STEP_NOCB(
    ux_sign_gas_rewards_1_step,
    bn_paging,
    {
      .title = "Baker",
      .text = (char *) global.signUpdateGasRewardsContext.gasBaker
    });
UX_STEP_NOCB(
    ux_sign_gas_rewards_2_step,
    bn_paging,
    {
      .title = "Finalization proof",
      .text = (char *) global.signUpdateGasRewardsContext.gasFinalization
    });
UX_STEP_NOCB(
    ux_sign_gas_rewards_3_step,
    bn_paging,
    {
        .title = "Account creation",
        .text = (char *) global.signUpdateGasRewardsContext.gasAccountCreation
    });
UX_STEP_NOCB(
    ux_sign_gas_rewards_4_step,
    bn_paging,
    {
      .title = "Chain update",
      .text = (char *) global.signUpdateGasRewardsContext.gasChainUpdate
    });
UX_STEP_CB(
    ux_sign_gas_rewards_5_step,
    pnn,
    buildAndSignTransactionHash(),
    {
      &C_icon_validate_14,
      "Sign",
      "transaction"
    });
UX_STEP_CB(
    ux_sign_gas_rewards_6_step,
    pnn,
    declineToSignTransaction(),
    {
      &C_icon_crossmark,
      "Decline to",
      "sign transaction"
    });
UX_FLOW(ux_sign_gas_rewards,
    &ux_sign_gas_rewards_0_step,
    &ux_sign_gas_rewards_1_step,
    &ux_sign_gas_rewards_2_step,
    &ux_sign_gas_rewards_3_step,
    &ux_sign_gas_rewards_4_step,
    &ux_sign_gas_rewards_5_step,
    &ux_sign_gas_rewards_6_step
);

void handleSignUpdateGasRewards(uint8_t *dataBuffer, volatile unsigned int *flags) {
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

    if (updateType != 8) {
        // Received an incorrect update type byte.
        THROW(0x6B01);
    }

    uint8_t fraction[10] = "/100000";

    // Baker GAS bytes
    uint32_t gasBaker = U4BE(dataBuffer, 0);
    int gasBakerLength = bin2dec(ctx->gasBaker, gasBaker);
    cx_hash((cx_hash_t *) &tx_state->hash, 0, dataBuffer, 4, NULL, 0);
    dataBuffer += 4;
    os_memmove(ctx->gasBaker + gasBakerLength, fraction, 10);

    // Finalization proof GAS bytes
    uint32_t gasFinalizationProof = U4BE(dataBuffer, 0);
    int gasFinalizationProofLength = bin2dec(ctx->gasFinalization, gasFinalizationProof);
    cx_hash((cx_hash_t *) &tx_state->hash, 0, dataBuffer, 4, NULL, 0);
    dataBuffer += 4;
    os_memmove(ctx->gasFinalization + gasFinalizationProofLength, fraction, 10);

    // Account creation GAS bytes
    uint32_t gasAccountCreation = U4BE(dataBuffer, 0);
    int gasAccountCreationLength = bin2dec(ctx->gasAccountCreation, gasAccountCreation);
    cx_hash((cx_hash_t *) &tx_state->hash, 0, dataBuffer, 4, NULL, 0);
    dataBuffer += 4;
    os_memmove(ctx->gasAccountCreation + gasAccountCreationLength, fraction, 10);

    // Chain update GAS bytes
    uint32_t gasChainUpdate = U4BE(dataBuffer, 0);
    int gasChainUpdateLength = bin2dec(ctx->gasChainUpdate, gasChainUpdate);
    cx_hash((cx_hash_t *) &tx_state->hash, 0, dataBuffer, 4, NULL, 0);
    dataBuffer += 4;
    os_memmove(ctx->gasChainUpdate + gasChainUpdateLength, fraction, 10);

    ux_flow_init(0, ux_sign_gas_rewards, NULL);
    *flags |= IO_ASYNCH_REPLY;
}
