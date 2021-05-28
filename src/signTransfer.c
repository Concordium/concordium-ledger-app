#include <os.h>
#include "util.h"
#include "sign.h"
#include "accountSenderView.h"
#include "base58check.h"

static signTransferContext_t *ctx = &global.signTransferContext;
static tx_state_t *tx_state = &global_tx_state;

UX_STEP_NOCB(
    ux_sign_flow_1_step,
    bn_paging,
    {
      "Amount (GTU)",
      (char *) global.signTransferContext.displayAmount,
    });
UX_STEP_NOCB(
    ux_sign_flow_2_step,
    bnnn_paging,
    {
      .title = "Recipient",
      .text = (char *) global.signTransferContext.displayStr
    });
UX_FLOW(ux_sign_flow,
    &ux_sign_flow_shared_review,
    &ux_sign_flow_account_sender_view,
    &ux_sign_flow_1_step,
    &ux_sign_flow_2_step,
    &ux_sign_flow_shared_sign,
    &ux_sign_flow_shared_decline
);

void handleSignTransfer(uint8_t *cdata, volatile unsigned int *flags) {
    // Parse the key derivation path, which should always be the first thing received
    // in a command to the Ledger application.
    cdata += parseKeyDerivationPath(cdata);

    // Initialize the hash that will be the hash of the whole transaction, which is what will be signed
    // if the user approves.
    cx_sha256_init(&tx_state->hash);
    cdata += hashAccountTransactionHeaderAndKind(cdata, TRANSFER);

    // Extract the recipient address and add to the hash.
    uint8_t toAddress[32];
    memmove(toAddress, cdata, 32);
    cdata += 32;
    cx_hash((cx_hash_t *) &tx_state->hash, 0, toAddress, 32, NULL, 0);

    // The recipient address is in a base58 format, so we need to encode it to be
    // able to display in a humand-readable way. This is written to ctx->displayStr as a string
    // so that it can be displayed.
    size_t outputSize = sizeof(ctx->displayStr);
    if (base58check_encode(toAddress, sizeof(toAddress), ctx->displayStr, &outputSize) != 0) {
      // The received address bytes are not a valid base58 encoding.
        THROW(SW_INVALID_TRANSACTION);  
    }
    ctx->displayStr[50] = '\0';

    // Build display value of the amount to transfer, and also add the bytes to the hash.
    uint64_t amount = U8BE(cdata, 0);
    amountToGtuDisplay(ctx->displayAmount, amount);
    cx_hash((cx_hash_t *) &tx_state->hash, 0, cdata, 8, NULL, 0);

    // Display the transaction information to the user (recipient address and amount to be sent).
    ux_flow_init(0, ux_sign_flow, NULL);

    // Tell the main process to wait for a button press.
    *flags |= IO_ASYNCH_REPLY;
}
