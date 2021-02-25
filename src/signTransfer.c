#include <os.h>
#include <os_io_seproxyhal.h>
#include "cx.h"
#include <stdint.h>
#include "menu.h"
#include "util.h"
#include <string.h>
#include "base58check.h"

static signTransferContext_t *ctx = &global.signTransferContext;
static tx_state_t *tx_state = &global.signTransferContext.tx_state;

void signTransferHash();

// UI definitions for displaying the transaction contents for verification before approval by
// the user.
UX_STEP_NOCB(
    ux_sign_flow_0_step,
    nn,
    {
      "Review",
      "transaction"
    });
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
UX_STEP_VALID(
    ux_sign_flow_3_step,
    pnn,
    signTransferHash(),
    {
      &C_icon_validate_14,
      "Sign",
      "transaction"
    });
UX_STEP_CB(
    ux_sign_flow_4_step,
    pnn,
    sendUserRejection(),
    {
      &C_icon_crossmark,
      "Decline to",
      "sign transaction"
    });
UX_FLOW(ux_sign_flow,
    &ux_sign_flow_0_step,
    &ux_sign_flow_1_step,
    &ux_sign_flow_2_step,
    &ux_sign_flow_3_step,
    &ux_sign_flow_4_step
);

// Function that is called when the user accepts signing the received transaction. It will use the private key
// to sign the hash of the transaction, and send the signature back to the computer.
void signTransferHash() {
    // Sign the transaction hash with the private key for the given account index.
    uint8_t signedHash[64];
    signTransactionHash(tx_state->transactionHash, signedHash);

    // Return the signature on the transaction hash to the computer.
    os_memmove(G_io_apdu_buffer, signedHash, sizeof(signedHash));
    sendSuccess(sizeof(signedHash));
}

// Constructs the SHA256 hash of the transaction bytes.
void buildTransferHash(uint8_t *dataBuffer) {
    // Initialize the hash that will be the hash of the whole transaction, which is what will be signed
    // if the user approves.
    cx_sha256_init(&tx_state->hash);

    // Add the transaction header to the hash. The transaction header is always 60 bytes.
    cx_hash((cx_hash_t *) &tx_state->hash, 0, dataBuffer, 60, NULL, 0);
    dataBuffer += 60;

    // Transaction payload/body comes right after the transaction header. First byte determines the transaction kind.
    uint8_t transactionKind = dataBuffer[0];
    if (transactionKind != TRANSFER) {
      THROW(SW_INVALID_TRANSACTION);
    }
    // Add transaction kind to the hash.
    cx_hash((cx_hash_t *) &tx_state->hash, 0, dataBuffer, 1, NULL, 0);
    dataBuffer += 1;

    // Extract the destination address and add to hash.
    uint8_t toAddress[32];
    os_memmove(toAddress, dataBuffer, 32);
    dataBuffer += 32;
    cx_hash((cx_hash_t *) &tx_state->hash, 0, toAddress, 32, NULL, 0);

    // Used to display recipient address.
    size_t outputSize = sizeof(ctx->displayStr);
    if (base58check_encode(toAddress, sizeof(toAddress), ctx->displayStr, &outputSize) != 0) {
      // The received address bytes are not a valid base58 encoding.
        THROW(SW_INVALID_TRANSACTION);  
    }
    ctx->displayStr[50] = '\0';

    // Used to display the amount being transferred.
    uint64_t amount = U8BE(dataBuffer, 0);
    bin2dec(ctx->displayAmount, amount);

    // Add transfer amount to the hash.
    cx_hash((cx_hash_t *) &tx_state->hash, 0, dataBuffer, 8, NULL, 0);

    // Build the hash and write to memory.
    cx_hash((cx_hash_t *) &tx_state->hash, CX_LAST, NULL, 0, tx_state->transactionHash, 32);
}

// Entry-point from the main class to the handler of signing simple transfers.
void handleSignTransfer(uint8_t *dataBuffer, uint16_t dataLength, volatile unsigned int *flags) {
    int bytesRead = parseKeyDerivationPath(dataBuffer);
    dataBuffer += bytesRead;

    // Calculate transaction hash. This function has the side effect that the values required to display
    // the transaction to the user are loaded. So it has to be run before initializing the ux_sign_flow.
    buildTransferHash(dataBuffer);

    // Display the transaction information to the user (recipient address and amount to be sent).
    ux_flow_init(0, ux_sign_flow, NULL);

    // Tell the main process to wait for a button press.
    *flags |= IO_ASYNCH_REPLY;
}
