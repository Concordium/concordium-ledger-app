#include <os.h>
#include <os_io_seproxyhal.h>
#include "cx.h"
#include <stdint.h>
#include "menu.h"
#include "util.h"
#include <string.h>
#include "sign.h"

static signUpdateMintDistribution_t *ctx = &global.signUpdateMintDistribution;
static tx_state_t *tx_state = &global_tx_state;

UX_STEP_NOCB(
    ux_sign_mint_rate_0_step,
    nn,
    {
      "Review",
      "transaction"
    });
UX_STEP_NOCB(
    ux_sign_mint_rate_1_step,
    bn_paging,
    {
      .title = "Mint rate",
      .text = (char *) global.signUpdateMintDistribution.mintRate
    });
UX_STEP_NOCB(
    ux_sign_mint_rate_2_step,
    bn_paging,
    {
      .title = "Baker reward",
      .text = (char *) global.signUpdateMintDistribution.bakerReward
    });
UX_STEP_NOCB(
    ux_sign_mint_rate_3_step,
    bn_paging,
    {
      .title = "Finalization reward",
      .text = (char *) global.signUpdateMintDistribution.finalizationReward
    });
UX_STEP_CB(
    ux_sign_mint_rate_4_step,
    pnn,
    buildAndSignTransactionHash(),
    {
      &C_icon_validate_14,
      "Sign",
      "transaction"
    });
UX_STEP_CB(
    ux_sign_mint_rate_5_step,
    pnn,
    declineToSignTransaction(),
    {
      &C_icon_crossmark,
      "Decline to",
      "sign transaction"
    });
UX_FLOW(ux_sign_mint_rate,
    &ux_sign_mint_rate_0_step,
    &ux_sign_mint_rate_1_step,
    &ux_sign_mint_rate_2_step,
    &ux_sign_mint_rate_3_step,
    &ux_sign_mint_rate_4_step,
    &ux_sign_mint_rate_5_step
);

void handleSignUpdateMintDistribution(uint8_t *dataBuffer, volatile unsigned int *flags) {
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

    if (updateType != 6) {
        // Received an incorrect update type byte.
        THROW(0x6B01);
    }

    // Mint rate consists of 4 bytes of mantissa, and a 1 byte exponent.
    uint32_t mintRateMantissa = U4BE(dataBuffer, 0);
    uint8_t mintRateExponent = dataBuffer[4];
    cx_hash((cx_hash_t *) &tx_state->hash, 0, dataBuffer, 5, NULL, 0);
    dataBuffer += 5;

    // Build display of the mint rate as 'mintRateMantissa*10^(-mintRateExponent)'
    int mintRateMantissaLength = bin2dec(ctx->mintRate, mintRateMantissa);
    uint8_t multiplication[6] = "*10^(-";
    os_memmove(ctx->mintRate + mintRateMantissaLength, multiplication, 6);
    int mintRateExponentLength = bin2dec(ctx->mintRate + mintRateMantissaLength + 6, mintRateExponent);
    uint8_t end_paranthesis[2] = ")\0";
    os_memmove(ctx->mintRate + mintRateMantissaLength + 6 + mintRateExponentLength, end_paranthesis, 2);

    // Baker reward
    uint8_t fraction[10] = "/100000";
    uint32_t bakerReward = U4BE(dataBuffer, 0);
    int bakerRewardLength = bin2dec(ctx->bakerReward, bakerReward);
    cx_hash((cx_hash_t *) &tx_state->hash, 0, dataBuffer, 4, NULL, 0);
    dataBuffer += 4;
    os_memmove(ctx->bakerReward + bakerRewardLength, fraction, 10);

    // Finalization reward
    uint32_t finalizationReward = U4BE(dataBuffer, 0);
    int finalizationRewardLength = bin2dec(ctx->finalizationReward, finalizationReward);
    cx_hash((cx_hash_t *) &tx_state->hash, 0, dataBuffer, 4, NULL, 0);
    dataBuffer += 4;
    os_memmove(ctx->finalizationReward + finalizationRewardLength, fraction, 10);

    ux_flow_init(0, ux_sign_mint_rate, NULL);
    *flags |= IO_ASYNCH_REPLY;
}
