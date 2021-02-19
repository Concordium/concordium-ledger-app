#include <os.h>
#include <os_io_seproxyhal.h>
#include "cx.h"
#include <stdint.h>
#include "menu.h"
#include "util.h"
#include <string.h>
#include "sign.h"

static signExchangeRateContext_t *ctx = &global.signExchangeRateContext;
static tx_state_t *tx_state = &global_tx_state;

UX_STEP_NOCB(
    ux_sign_exchange_rate_0_step,
    nn,
    {
      "Review",
      "transaction"
    });
UX_STEP_NOCB(
    ux_sign_exchange_rate_1_step,
    bn_paging,
    {
      .title = (char *) global.signExchangeRateContext.type,
      .text = (char *) global.signExchangeRateContext.ratio
    });
UX_STEP_CB(
    ux_sign_exchange_rate_2_step,
    pnn,
    buildAndSignTransactionHash(),
    {
      &C_icon_validate_14,
      "Sign",
      "transaction"
    });
UX_STEP_CB(
    ux_sign_exchange_rate_3_step,
    pnn,
    declineToSignTransaction(),
    {
      &C_icon_crossmark,
      "Decline to",
      "sign transaction"
    });
UX_FLOW(ux_sign_exchange_rate,
    &ux_sign_exchange_rate_0_step,
    &ux_sign_exchange_rate_1_step,
    &ux_sign_exchange_rate_2_step,
    &ux_sign_exchange_rate_3_step
);

// Handles signing update transactions for updating an exchange rate. The signing method
// is shared as the update payloads (except for the type declaration) are identical for 
// these types of transactions.
//
// Currently it supports two update transactions:
//  - UpdateEuroPerEnergy
//  - UpdateMicroGTUPerEuro
void handleSignUpdateExchangeRate(uint8_t *dataBuffer, volatile unsigned int *flags) {
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

    if (updateType == 3) {
        os_memmove(ctx->type, "Euro per energy", 15);
        ctx->type[15] = '\0';
    } else if (updateType == 4) {
        os_memmove(ctx->type, "uGTU per Euro", 13);
        ctx->type[13] = '\0';
    } else {
        THROW(0x6B01);  // Received an unsupported exchange rate transaction.
    }

    // Numerator is the first 8 bytes.
    uint64_t numerator = U8BE(dataBuffer, 0);
    int numeratorLength = bin2dec(ctx->ratio, numerator);
    cx_hash((cx_hash_t *) &tx_state->hash, 0, dataBuffer, 8, NULL, 0);
    dataBuffer += 8;

    uint8_t slash[3] = " / ";
    os_memmove(ctx->ratio + numeratorLength, slash, 3);

    // Denominator is the last 8 bytes.
    uint64_t denominator = U8BE(dataBuffer, 0);
    bin2dec(ctx->ratio + numeratorLength + sizeof(slash), denominator);
    cx_hash((cx_hash_t *) &tx_state->hash, 0, dataBuffer, 8, NULL, 0);
    dataBuffer += 8;

    ux_flow_init(0, ux_sign_exchange_rate, NULL);
    *flags |= IO_ASYNCH_REPLY;
}
