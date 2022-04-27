#include <os.h>

#include "base58check.h"
#include "responseCodes.h"
#include "sign.h"
#include "util.h"

static signUpdateFoundationAccountContext_t *ctx = &global.signUpdateFoundationAccountContext;
static tx_state_t *tx_state = &global_tx_state;

UX_STEP_NOCB(
    ux_sign_foundation_account_address_1_step,
    bnnn_paging,
    {.title = "Foundation acc.", .text = (char *) global.signUpdateFoundationAccountContext.foundationAccountAddress});
UX_FLOW(
    ux_sign_foundation_account_address,
    &ux_sign_flow_shared_review,
    &ux_sign_foundation_account_address_1_step,
    &ux_sign_flow_shared_sign,
    &ux_sign_flow_shared_decline);

void handleSignUpdateFoundationAccount(uint8_t *cdata, volatile unsigned int *flags) {
    cdata += parseKeyDerivationPath(cdata);

    cx_sha256_init(&tx_state->hash);
    cdata += hashUpdateHeaderAndType(cdata, UPDATE_TYPE_FOUNDATION_ACCOUNT);

    // The foundation account address is 32 bytes.
    uint8_t foundationAccount[32];
    memmove(foundationAccount, cdata, 32);
    cx_hash((cx_hash_t *) &tx_state->hash, 0, foundationAccount, 32, NULL, 0);

    // Used to display the foundation account address
    size_t outputSize = sizeof(ctx->foundationAccountAddress);
    if (base58check_encode(foundationAccount, sizeof(foundationAccount), ctx->foundationAccountAddress, &outputSize) !=
        0) {
        // The received address bytes were not valid a valid base58 encoding, so the transaction is invalid.
        THROW(ERROR_INVALID_TRANSACTION);
    }
    ctx->foundationAccountAddress[55] = '\0';

    ux_flow_init(0, ux_sign_foundation_account_address, NULL);
    *flags |= IO_ASYNCH_REPLY;
}
