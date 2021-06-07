#include <os.h>
#include "util.h"
#include "accountSenderView.h"
#include "sign.h"
#include "base58check.h"

static signEncryptedAmountToTransfer_t *ctx = &global.signEncryptedAmountToTransfer;
static tx_state_t *tx_state = &global_tx_state;

// UI for displaying encrypted transfer transaction. It only shows the user the recipient address
// as the amounts are encrypted and can't be validated by the user.
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
    sendSuccessNoIdle(),
    {
      .title = "Recipient",
      .text = (char *) global.signEncryptedAmountToTransfer.to
    });
UX_FLOW(ux_sign_encrypted_amount_transfer,
    &ux_sign_flow_shared_review,
    &ux_sign_encrypted_amount_transfer_1_step,
    &ux_sign_flow_account_sender_view,
    &ux_sign_encrypted_amount_transfer_2_step,
    &ux_sign_flow_shared_sign,
    &ux_sign_flow_shared_decline
);

#define P1_INITIAL                              0x00
#define P1_REMAINING_AMOUNT                     0x01
#define P1_TRANSFER_AMOUNT_AGG_INDEX_PROOF_SIZE 0x02
#define P1_PROOF                                0x03

void handleSignEncryptedAmountTransfer(uint8_t *cdata, uint8_t p1, uint8_t dataLength, volatile unsigned int *flags) {
    if (p1 == P1_INITIAL && tx_state->initialized == false) {
        cdata += parseKeyDerivationPath(cdata);
        cx_sha256_init(&tx_state->hash);
        tx_state->initialized = true;
        cdata += hashAccountTransactionHeaderAndKind(cdata, ENCRYPTED_AMOUNT_TRANSFER);

        // To account address
        uint8_t toAddress[32];
        memmove(toAddress, cdata, 32);
        cdata += 32;
        cx_hash((cx_hash_t *) &tx_state->hash, 0, toAddress, 32, NULL, 0);
        size_t outputSize = sizeof(ctx->to);
        if (base58check_encode(toAddress, sizeof(toAddress), ctx->to, &outputSize) != 0) {
            // The received address bytes are not valid a valid base58 encoding.
            THROW(SW_INVALID_TRANSACTION);
        }
        ctx->to[50] = '\0';

        ctx->state = TX_ENCRYPTED_AMOUNT_TRANSFER_REMAINING_AMOUNT;
        sendSuccessNoIdle();
    } else if (p1 == P1_REMAINING_AMOUNT && ctx->state == TX_ENCRYPTED_AMOUNT_TRANSFER_REMAINING_AMOUNT) {
        // Hash remaining amount. Remaining amount is encrypted, and so we cannot display it.
        cx_hash((cx_hash_t *) &tx_state->hash, 0, cdata, 192, NULL, 0);
        ctx->state = TX_ENCRYPTED_AMOUNT_TRANSFER_TRANSFER_AMOUNT;
        sendSuccessNoIdle();
    } else if (p1 == P1_TRANSFER_AMOUNT_AGG_INDEX_PROOF_SIZE && ctx->state == TX_ENCRYPTED_AMOUNT_TRANSFER_TRANSFER_AMOUNT) {
        // Hash transfer amound and agg index. Transfer amount is encrypted, and so we cannot display it.
        cx_hash((cx_hash_t *) &tx_state->hash, 0, cdata, 200, NULL, 0);
        cdata += 200;

        // Save proof size so that we know when we are done processing the 
        // proof bytes that we are going to receive.
        ctx->proofSize = U2BE(cdata, 0);

        ctx->state = TX_ENCRYPTED_AMOUNT_TRANSFER_PROOFS;
        sendSuccessNoIdle();
    } else if (p1 == P1_PROOF && ctx->state == TX_ENCRYPTED_AMOUNT_TRANSFER_PROOFS) { 
        cx_hash((cx_hash_t *) &tx_state->hash, 0, cdata, dataLength, NULL, 0);
        ctx->proofSize -= dataLength;

        if (ctx->proofSize == 0) {
            // We have received all proof bytes, continue to signing flow.
            ux_flow_init(0, ux_sign_encrypted_amount_transfer, NULL);
            *flags |= IO_ASYNCH_REPLY;
        } else if (ctx->proofSize < 0) {
            // We received more proof bytes than expected.
            THROW(SW_INVALID_STATE);    
        } else {
            // There are more bytes to be received. Ask the computer for more.
            sendSuccessNoIdle();
        }
    } else {
        THROW(SW_INVALID_STATE);
    }
}
