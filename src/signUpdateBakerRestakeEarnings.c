#include <os.h>
#include "util.h"
#include "sign.h"

static sigUpdateBakerRestakeEarningsContext_t *ctx = &global.signUpdateBakerRestakeEarnings;
static tx_state_t *tx_state = &global_tx_state;

UX_STEP_NOCB(
    ux_sign_update_baker_restake_earnings_1_step,
    bn_paging,
    {
        .title = "Restake earnings",
        .text = (char *) global.signUpdateBakerRestakeEarnings.restake
    });
UX_FLOW(ux_sign_update_baker_restake_earnings,
    &ux_sign_flow_shared_review,
    &ux_sign_update_baker_restake_earnings_1_step,
    &ux_sign_flow_shared_sign,
    &ux_sign_flow_shared_decline
);

void handleSignUpdateBakerRestakeEarnings(uint8_t *dataBuffer, volatile unsigned int *flags) {
    int bytesRead = parseKeyDerivationPath(dataBuffer);
    dataBuffer += bytesRead;

    cx_sha256_init(&tx_state->hash);
    cx_hash((cx_hash_t *) &tx_state->hash, 0, dataBuffer, ACCOUNT_TRANSACTION_HEADER_LENGTH + 1, NULL, 0);
    uint8_t transactionKind = dataBuffer[ACCOUNT_TRANSACTION_HEADER_LENGTH];
    if (transactionKind != UPDATE_BAKER_STAKE_EARNINGS) {
        THROW(SW_INVALID_TRANSACTION);
    }
    dataBuffer += (ACCOUNT_TRANSACTION_HEADER_LENGTH + 1);

    // Parse the bool (as 1 byte) of whether to restake earnings.
    uint8_t restakeEarnings = dataBuffer[0];
    cx_hash((cx_hash_t *) &tx_state->hash, 0, dataBuffer, 1, NULL, 0);
    if (restakeEarnings == 0) {
        os_memmove(ctx->restake, "No\0", 3);
    } else if (restakeEarnings == 1) {
        os_memmove(ctx->restake, "Yes\0", 4);
    } else {
        THROW(SW_INVALID_TRANSACTION);
    }

    ux_flow_init(0, ux_sign_update_baker_restake_earnings, NULL);
    *flags |= IO_ASYNCH_REPLY;
}
