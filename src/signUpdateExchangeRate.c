#include <os.h>
#include "util.h"
#include "sign.h"

static signExchangeRateContext_t *ctx = &global.signExchangeRateContext;
static tx_state_t *tx_state = &global_tx_state;

UX_STEP_NOCB(
    ux_sign_exchange_rate_1_step,
    bn_paging,
    {
      .title = (char *) global.signExchangeRateContext.type,
      .text = (char *) global.signExchangeRateContext.ratio
    });
UX_FLOW(ux_sign_exchange_rate,
    &ux_sign_flow_shared_review,
    &ux_sign_exchange_rate_1_step,
    &ux_sign_flow_shared_sign,
    &ux_sign_flow_shared_decline
);

// Handles signing update transactions for updating an exchange rate. The signing method
// is shared as the update payloads (except for the type declaration) are identical for 
// these types of transactions.
//
// Currently it supports two update transactions:
//  - UpdateEuroPerEnergy
//  - UpdateMicroGTUPerEuro
void handleSignUpdateExchangeRate(uint8_t *cdata, volatile unsigned int *flags) {
    int bytesRead = parseKeyDerivationPath(cdata);
    cdata += bytesRead;

    cx_sha256_init(&tx_state->hash);
    // Note that this method cannot easily use hashUpdateHeaderAndType from util.h, as 
    // the UI is built based on the specific update type.
    cx_hash((cx_hash_t *) &tx_state->hash, 0, cdata, UPDATE_HEADER_LENGTH, NULL, 0);
    cdata += UPDATE_HEADER_LENGTH;
    uint8_t updateType = cdata[0];
    cx_hash((cx_hash_t *) &tx_state->hash, 0, cdata, 1, NULL, 0);
    cdata += 1;

    if (updateType == 3) {
        memmove(ctx->type, "Euro per energy", 15);
        ctx->type[15] = '\0';
    } else if (updateType == 4) {
        memmove(ctx->type, "uGTU per Euro", 13);
        ctx->type[13] = '\0';
    } else {
        // Received an unsupported exchange rate transaction.
        THROW(SW_INVALID_TRANSACTION);
    }

    // Numerator is the first 8 bytes.
    uint64_t numerator = U8BE(cdata, 0);
    int numeratorLength = numberToText(ctx->ratio, numerator);
    cx_hash((cx_hash_t *) &tx_state->hash, 0, cdata, 8, NULL, 0);
    cdata += 8;

    uint8_t slash[3] = " / ";
    memmove(ctx->ratio + numeratorLength, slash, 3);

    // Denominator is the last 8 bytes.
    uint64_t denominator = U8BE(cdata, 0);
    bin2dec(ctx->ratio + numeratorLength + sizeof(slash), denominator);
    cx_hash((cx_hash_t *) &tx_state->hash, 0, cdata, 8, NULL, 0);
    cdata += 8;

    ux_flow_init(0, ux_sign_exchange_rate, NULL);
    *flags |= IO_ASYNCH_REPLY;
}
