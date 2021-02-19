#include <os.h>
#include <os_io_seproxyhal.h>
#include "cx.h"
#include <stdint.h>
#include "menu.h"
#include "util.h"
#include <string.h>
#include "sign.h"

static signUpdateFoundationAccountContext_t *ctx = &global.signUpdateFoundationAccountContext;
static tx_state_t *tx_state = &global_tx_state;

UX_STEP_NOCB(
    ux_sign_foundation_account_address_0_step,
    nn,
    {
      "Review",
      "transaction"
    });
UX_STEP_NOCB(
    ux_sign_foundation_account_address_1_step,
    bn_paging,
    {
      .title = "Foundation acc.",
      .text = (char *) global.signUpdateFoundationAccountContext.foundationAccountAddress
    });
UX_STEP_CB(
    ux_sign_foundation_account_address_2_step,
    pnn,
    buildAndSignTransactionHash(),
    {
      &C_icon_validate_14,
      "Sign",
      "transaction"
    });
UX_STEP_CB(
    ux_sign_foundation_account_address_3_step,
    pnn,
    declineToSignTransaction(),
    {
      &C_icon_crossmark,
      "Decline to",
      "sign transaction"
    });
UX_FLOW(ux_sign_foundation_account_address,
    &ux_sign_foundation_account_address_0_step,
    &ux_sign_foundation_account_address_1_step,
    &ux_sign_foundation_account_address_2_step,
    &ux_sign_foundation_account_address_3_step
);

void handleSignUpdateFoundationAccount(uint8_t *dataBuffer, volatile unsigned int *flags) {
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

    if (updateType != 5) {
        // Received an incorrect update type byte.
        THROW(0x6B01);
    }

    // The foundation account address is 32 bytes.
    uint8_t foundationAccount[32];
    os_memmove(foundationAccount, dataBuffer, 32);
    dataBuffer += 32;
    cx_hash((cx_hash_t *) &tx_state->hash, 0, foundationAccount, 32, NULL, 0);

    // Used to display the foundation account address
    size_t outputSize = sizeof(ctx->foundationAccountAddress);
    if (base58check_encode(foundationAccount, sizeof(foundationAccount), ctx->foundationAccountAddress, &outputSize) != 0) {
        THROW(0x6B01);  // The received address bytes are not valid a valid base58 encoding.
    }
    ctx->foundationAccountAddress[50] = '\0';

    ux_flow_init(0, ux_sign_foundation_account_address, NULL);
    *flags |= IO_ASYNCH_REPLY;
}
