#include <os.h>
#include <os_io_seproxyhal.h>
#include "cx.h"
#include <stdint.h>
#include "menu.h"
#include "util.h"
#include <string.h>
#include "base58check.h"
#include <stdio.h>
#include "sign.h"

static signEncryptedAmountToTransfer_t *ctx = &global.signEncryptedAmountToTransfer;
static tx_state_t *tx_state = &global_tx_state;

// UI for displaying encrypted transfer transaction. It only shows the user the recipient address
// as the amounts are encrypted and can't be validated by the user.
UX_STEP_NOCB(
    ux_sign_encrypted_amount_transfer_0_step,
    nn,
    {
      "Review",
      "transaction"
    });
UX_STEP_NOCB(
    ux_sign_encrypted_amount_transfer_1_step,
    nn,
    {
      "Encrypted amount",
      "transfer"
    });
UX_STEP_CB(
    ux_sign_encrypted_amount_transfer_2_step,
    bn_paging,
    sendSuccessNoIdle(0),
    {
      .title = "Recipient",
      .text = (char *) global.signEncryptedAmountToTransfer.to
    });
UX_FLOW(ux_sign_encrypted_amount_transfer,
    &ux_sign_encrypted_amount_transfer_0_step,
    &ux_sign_encrypted_amount_transfer_1_step,
    &ux_sign_encrypted_amount_transfer_2_step
);

#define P1_INITIAL              0x00        // Key path, transaction header, kind and to address, encrypted amounts and proof size.
#define P1_PROOF                0x01        // Proof bytes.

void handleSignEncryptedAmountTransfer(uint8_t *dataBuffer, uint8_t p1, uint8_t dataLength, volatile unsigned int *flags) {
    if (p1 == P1_INITIAL) {    
        int bytesRead = parseKeyDerivationPath(dataBuffer);
        dataBuffer += bytesRead;

        // Initialize hashing and add transaction header and transaction kind to the hash.
        cx_sha256_init(&tx_state->hash);
        cx_hash((cx_hash_t *) &tx_state->hash, 0, dataBuffer, ACCOUNT_TRANSACTION_HEADER_LENGTH + 1, NULL, 0);
        dataBuffer += ACCOUNT_TRANSACTION_HEADER_LENGTH + 1;

        // // To account address
        // // FIXME: Could be generalized as we use this in multiple places.
        uint8_t toAddress[32];
        os_memmove(toAddress, dataBuffer, 32);
        dataBuffer += 32;
        cx_hash((cx_hash_t *) &tx_state->hash, 0, toAddress, 32, NULL, 0);
        size_t outputSize = sizeof(ctx->to);
        if (base58check_encode(toAddress, sizeof(toAddress), ctx->to, &outputSize) != 0) {
            THROW(0x6B01);  // The received address bytes are not valid a valid base58 encoding.
        }
        ctx->to[50] = '\0';

        // Hash amounts, encrypted amount agg index and size of proof in bytes.
        cx_hash((cx_hash_t *) &tx_state->hash, 0, dataBuffer, 26, NULL, 0);

        // Save proof size so that we know when we are done processing the 
        // proof bytes that we are going to receive.
        ctx->proofSize = U2BE(dataBuffer, 24);

    ux_flow_init(0, ux_sign_encrypted_amount_transfer, NULL);
    *flags |= IO_ASYNCH_REPLY;
    } else if (p1 == P1_PROOF) { 
        cx_hash((cx_hash_t *) &tx_state->hash, 0, dataBuffer, dataLength, NULL, 0);
        ctx->proofSize -= dataLength;

        if (ctx->proofSize == 0) {
            // We have received all proof bytes, continue to signing flow.
            ux_flow_init(0, ux_sign_flow_shared, NULL);
            *flags |= IO_ASYNCH_REPLY;
        } else if (ctx->proofSize < 0) {
            // We received more proof bytes than expected.
            THROW(SW_INVALID_STATE);    
        } else {
            // There are more bytes to be received. Ask the computer for more.
            sendSuccessNoIdle(0);
        }
    } else {
        THROW(SW_INVALID_PARAM);
    }
}
