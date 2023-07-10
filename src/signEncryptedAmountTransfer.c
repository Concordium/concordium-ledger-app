#include <os.h>

#include "accountSenderView.h"
#include "base58check.h"
#include "displayCbor.h"
#include "responseCodes.h"
#include "sign.h"
#include "util.h"

static signEncryptedAmountToTransfer_t *ctx = &global.withDataBlob.signEncryptedAmountToTransfer;
static cborContext_t *memo_ctx = &global.withDataBlob.cborContext;
static tx_state_t *tx_state = &global_tx_state;
const ux_flow_step_t *ux_sign_encrypted_amount_transfer[8];

// UI for displaying encrypted transfer transaction. It only shows the user the recipient address
// as the amounts are encrypted and can't be validated by the user.
UX_STEP_NOCB(ux_sign_encrypted_amount_transfer_1_step, nn, {"Shielded", "transfer"});
UX_STEP_NOCB(
    ux_sign_encrypted_amount_transfer_2_step,
    bnnn_paging,
    {.title = "Recipient", .text = (char *) global.withDataBlob.signEncryptedAmountToTransfer.to});

void startEncryptedTransferDisplay(bool displayMemo) {
    uint8_t index = 0;

    ux_sign_encrypted_amount_transfer[index++] = &ux_sign_flow_shared_review;
    ux_sign_encrypted_amount_transfer[index++] = &ux_sign_encrypted_amount_transfer_1_step;
    ux_sign_encrypted_amount_transfer[index++] = &ux_sign_flow_account_sender_view;
    ux_sign_encrypted_amount_transfer[index++] = &ux_sign_encrypted_amount_transfer_2_step;

    if (displayMemo) {
        ux_sign_encrypted_amount_transfer[index++] = &ux_display_memo_step_nocb;
    }

    ux_sign_encrypted_amount_transfer[index++] = &ux_sign_flow_shared_sign;
    ux_sign_encrypted_amount_transfer[index++] = &ux_sign_flow_shared_decline;

    ux_sign_encrypted_amount_transfer[index++] = FLOW_END_STEP;
    ux_flow_init(0, ux_sign_encrypted_amount_transfer, NULL);
}

#define P1_INITIAL                              0x00
#define P1_REMAINING_AMOUNT                     0x01
#define P1_TRANSFER_AMOUNT_AGG_INDEX_PROOF_SIZE 0x02
#define P1_PROOF                                0x03
#define P1_INITIAL_WITH_MEMO                    0x04
#define P1_MEMO                                 0x05

void handleRemainingAmount(uint8_t *cdata) {
    // Hash remaining amount. Remaining amount is encrypted, and so we cannot display it.
    updateHash((cx_hash_t *) &tx_state->hash, cdata, 192);
    ctx->state = TX_ENCRYPTED_AMOUNT_TRANSFER_TRANSFER_AMOUNT;
    sendSuccessNoIdle();
}

void handleTransferAmountAggIndexProofSize(uint8_t *cdata) {
    // Hash transfer amount and agg index. Transfer amount is encrypted, and so we cannot display it.
    updateHash((cx_hash_t *) &tx_state->hash, cdata, 200);
    cdata += 200;

    // Save proof size so that we know when we are done processing the
    // proof bytes that we are going to receive.
    ctx->proofSize = U2BE(cdata, 0);

    ctx->state = TX_ENCRYPTED_AMOUNT_TRANSFER_PROOFS;
    sendSuccessNoIdle();
}

void handleProofs(uint8_t *cdata, uint8_t dataLength, volatile unsigned int *flags, bool displayMemo) {
    updateHash((cx_hash_t *) &tx_state->hash, cdata, dataLength);
    ctx->proofSize -= dataLength;

    if (ctx->proofSize == 0) {
        // We have received all proof bytes, continue to signing flow.
        startEncryptedTransferDisplay(displayMemo);
        *flags |= IO_ASYNCH_REPLY;
    } else if (ctx->proofSize < 0) {
        // We received more proof bytes than expected.
        THROW(ERROR_INVALID_STATE);
    } else {
        // There are more bytes to be received. Ask the computer for more.
        sendSuccessNoIdle();
    }
}

void finishMemoEncrypted() {
    ctx->state = TX_ENCRYPTED_AMOUNT_TRANSFER_REMAINING_AMOUNT;
    sendSuccessNoIdle();
}

void handleSignEncryptedAmountTransferWithMemo(
    uint8_t *cdata,
    uint8_t p1,
    uint8_t dataLength,
    volatile unsigned int *flags,
    bool isInitialCall) {
    if (isInitialCall) {
        ctx->state = TX_ENCRYPTED_AMOUNT_TRANSFER_INITIAL;
    }

    if (p1 == P1_INITIAL_WITH_MEMO && ctx->state == TX_ENCRYPTED_AMOUNT_TRANSFER_INITIAL) {
        cdata += handleHeaderAndToAddress(cdata, ENCRYPTED_AMOUNT_TRANSFER_WITH_MEMO, ctx->to, sizeof(ctx->to));

        // Hash memo length
        memo_ctx->cborLength = U2BE(cdata, 0);
        if (memo_ctx->cborLength > MAX_MEMO_SIZE) {
            THROW(ERROR_INVALID_PARAM);
        }

        updateHash((cx_hash_t *) &tx_state->hash, cdata, 2);

        ctx->state = TX_ENCRYPTED_AMOUNT_TRANSFER_MEMO_START;
        sendSuccessNoIdle();
    } else if (p1 == P1_MEMO && ctx->state == TX_ENCRYPTED_AMOUNT_TRANSFER_MEMO_START) {
        updateHash((cx_hash_t *) &tx_state->hash, cdata, dataLength);

        // Read initial part of memo and then display it:
        readCborInitial(cdata, dataLength);

        if (memo_ctx->cborLength == 0) {
            finishMemoEncrypted();
        } else {
            ctx->state = TX_ENCRYPTED_AMOUNT_TRANSFER_MEMO;
            sendSuccessNoIdle();
        }
    } else if (p1 == P1_MEMO && ctx->state == TX_ENCRYPTED_AMOUNT_TRANSFER_MEMO) {
        updateHash((cx_hash_t *) &tx_state->hash, cdata, dataLength);

        // Read current part of memo and then display it:
        readCborContent(cdata, dataLength);

        if (memo_ctx->cborLength != 0) {
            // The memo size is <=256 bytes, so we should always have received the complete memo by this point;
            THROW(ERROR_INVALID_STATE);
        }

        finishMemoEncrypted();
    } else if (p1 == P1_REMAINING_AMOUNT && ctx->state == TX_ENCRYPTED_AMOUNT_TRANSFER_REMAINING_AMOUNT) {
        handleRemainingAmount(cdata);
    } else if (
        p1 == P1_TRANSFER_AMOUNT_AGG_INDEX_PROOF_SIZE && ctx->state == TX_ENCRYPTED_AMOUNT_TRANSFER_TRANSFER_AMOUNT) {
        handleTransferAmountAggIndexProofSize(cdata);
    } else if (p1 == P1_PROOF && ctx->state == TX_ENCRYPTED_AMOUNT_TRANSFER_PROOFS) {
        handleProofs(cdata, dataLength, flags, true);
    } else {
        THROW(ERROR_INVALID_STATE);
    }
}

void handleSignEncryptedAmountTransfer(
    uint8_t *cdata,
    uint8_t p1,
    uint8_t dataLength,
    volatile unsigned int *flags,
    bool isInitialCall) {
    if (isInitialCall) {
        ctx->state = TX_ENCRYPTED_AMOUNT_TRANSFER_INITIAL;
    }
    if (p1 == P1_INITIAL && ctx->state == TX_ENCRYPTED_AMOUNT_TRANSFER_INITIAL) {
        handleHeaderAndToAddress(cdata, ENCRYPTED_AMOUNT_TRANSFER, ctx->to, sizeof(ctx->to));
        ctx->state = TX_ENCRYPTED_AMOUNT_TRANSFER_REMAINING_AMOUNT;
        sendSuccessNoIdle();
    } else if (p1 == P1_REMAINING_AMOUNT && ctx->state == TX_ENCRYPTED_AMOUNT_TRANSFER_REMAINING_AMOUNT) {
        handleRemainingAmount(cdata);
    } else if (
        p1 == P1_TRANSFER_AMOUNT_AGG_INDEX_PROOF_SIZE && ctx->state == TX_ENCRYPTED_AMOUNT_TRANSFER_TRANSFER_AMOUNT) {
        handleTransferAmountAggIndexProofSize(cdata);
    } else if (p1 == P1_PROOF && ctx->state == TX_ENCRYPTED_AMOUNT_TRANSFER_PROOFS) {
        handleProofs(cdata, dataLength, flags, false);
    } else {
        THROW(ERROR_INVALID_STATE);
    }
}
