#include <os.h>
#include <os_io_seproxyhal.h>
#include "cx.h"
#include <stdint.h>
#include "menu.h"
#include "util.h"
#include <string.h>
#include "base58check.h"
#include "sign.h"

static signTransferContext_t *ctx = &global.signTransferContext;
static tx_state_t *tx_state = &global_tx_state;

UX_STEP_NOCB(
    ux_sign_flow_1_step,
    bn,
    {
      "Amount (uGTU)",
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
    &ux_sign_flow_1_step,
    &ux_sign_flow_2_step,
    &ux_sign_flow_shared_sign,
    &ux_sign_flow_shared_decline
);

void buildTransferHash(uint8_t *cdata) {
    // Initialize the hash that will be the hash of the whole transaction, which is what will be signed
    // if the user approves.
    cx_sha256_init(&tx_state->hash);

    // Add the transaction header to the hash. The transaction header is always 60 bytes.
    cx_hash((cx_hash_t *) &tx_state->hash, 0, cdata, ACCOUNT_TRANSACTION_HEADER_LENGTH, NULL, 0);
    cdata += ACCOUNT_TRANSACTION_HEADER_LENGTH;

    // Transaction payload/body comes right after the transaction header. First byte determines the transaction kind.
    uint8_t transactionKind = cdata[0];
    if (transactionKind != TRANSFER) {
      THROW(SW_INVALID_TRANSACTION);
    }
    // Add transaction kind to the hash.
    cx_hash((cx_hash_t *) &tx_state->hash, 0, cdata, 1, NULL, 0);
    cdata += 1;

    // Extract the destination address and add to hash.
    uint8_t toAddress[32];
    os_memmove(toAddress, cdata, 32);
    cdata += 32;
    cx_hash((cx_hash_t *) &tx_state->hash, 0, toAddress, 32, NULL, 0);

    // Used to display recipient address.
    size_t outputSize = sizeof(ctx->displayStr);
    if (base58check_encode(toAddress, sizeof(toAddress), ctx->displayStr, &outputSize) != 0) {
      // The received address bytes are not a valid base58 encoding.
        THROW(SW_INVALID_TRANSACTION);  
    }
    ctx->displayStr[50] = '\0';

    // Used to display the amount being transferred.
    uint64_t amount = U8BE(cdata, 0);
    bin2dec(ctx->displayAmount, amount);

    // Add transfer amount to the hash.
    cx_hash((cx_hash_t *) &tx_state->hash, 0, cdata, 8, NULL, 0);
}

void handleSignTransfer(uint8_t *cdata, volatile unsigned int *flags) {
    int bytesRead = parseKeyDerivationPath(cdata);
    cdata += bytesRead;

    // Calculate transaction hash. This function has the side effect that the values required to display
    // the transaction to the user are loaded. So it has to be run before initializing the ux_sign_flow.
    buildTransferHash(cdata);

    // Display the transaction information to the user (recipient address and amount to be sent).
    ux_flow_init(0, ux_sign_flow, NULL);

    // Tell the main process to wait for a button press.
    *flags |= IO_ASYNCH_REPLY;
}
