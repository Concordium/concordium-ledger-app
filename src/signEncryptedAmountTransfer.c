#include <os.h>

#include "accountSenderView.h"
#include "base58check.h"
#include "memo.h"
#include "responseCodes.h"
#include "sign.h"
#include "util.h"

static signEncryptedAmountToTransfer_t *ctx = &global.withMemo.signEncryptedAmountToTransfer;
static memoContext_t *memo_ctx = &global.withMemo.memoContext;
static tx_state_t *tx_state = &global_tx_state;

// UI for displaying encrypted transfer transaction. It only shows the user the recipient address
// as the amounts are encrypted and can't be validated by the user.
UX_STEP_NOCB(ux_sign_encrypted_amount_transfer_1_step, nn, {"Shielded", "transfer"});
UX_STEP_CB(
    ux_sign_encrypted_amount_transfer_2_step,
    bnnn_paging,
    sendSuccessNoIdle(),
    {.title = "Recipient", .text = (char *) global.withMemo.signEncryptedAmountToTransfer.to});
UX_FLOW(
    ux_sign_encrypted_amount_transfer,
    &ux_sign_flow_shared_review,
    &ux_sign_encrypted_amount_transfer_1_step,
    &ux_sign_flow_account_sender_view,
    &ux_sign_encrypted_amount_transfer_2_step,
    &ux_sign_flow_shared_sign,
    &ux_sign_flow_shared_decline);

UX_FLOW(
    ux_sign_encrypted_amount_transfer_initial,
    &ux_sign_flow_shared_review,
    &ux_sign_encrypted_amount_transfer_1_step,
    &ux_sign_flow_account_sender_view,
    &ux_sign_encrypted_amount_transfer_2_step);

#define P1_INITIAL                              0x00
#define P1_REMAINING_AMOUNT                     0x01
#define P1_TRANSFER_AMOUNT_AGG_INDEX_PROOF_SIZE 0x02
#define P1_PROOF                                0x03
#define P1_INITIAL_WITH_MEMO                    0x04
#define P1_MEMO                                 0x05

void handleRemainingAmount(uint8_t *cdata) {
    // Hash remaining amount. Remaining amount is encrypted, and so we cannot display it.
    cx_hash((cx_hash_t *) &tx_state->hash, 0, cdata, 192, NULL, 0);
    ctx->state = TX_ENCRYPTED_AMOUNT_TRANSFER_TRANSFER_AMOUNT;
    sendSuccessNoIdle();
}

void handleTransferAmountAggIndexProofSize(uint8_t *cdata) {
    // Hash transfer amount and agg index. Transfer amount is encrypted, and so we cannot display it.
    cx_hash((cx_hash_t *) &tx_state->hash, 0, cdata, 200, NULL, 0);
    cdata += 200;

    // Save proof size so that we know when we are done processing the
    // proof bytes that we are going to receive.
    ctx->proofSize = U2BE(cdata, 0);

    ctx->state = TX_ENCRYPTED_AMOUNT_TRANSFER_PROOFS;
    sendSuccessNoIdle();
}

void handleProofs(
    uint8_t *cdata,
    uint8_t dataLength,
    volatile unsigned int *flags,
    const ux_flow_step_t *const *finishedFlow) {
    cx_hash((cx_hash_t *) &tx_state->hash, 0, cdata, dataLength, NULL, 0);
    ctx->proofSize -= dataLength;

    if (ctx->proofSize == 0) {
        // We have received all proof bytes, continue to signing flow.
        ux_flow_init(0, finishedFlow, NULL);
        *flags |= IO_ASYNCH_REPLY;
    } else if (ctx->proofSize < 0) {
        // We received more proof bytes than expected.
        THROW(ERROR_INVALID_STATE);
    } else {
        // There are more bytes to be received. Ask the computer for more.
        sendSuccessNoIdle();
    }
}

void finishMemoEncrypted(volatile unsigned int *flags) {
    ctx->state = TX_ENCRYPTED_AMOUNT_TRANSFER_REMAINING_AMOUNT;
    ux_flow_init(0, ux_sign_transfer_memo, NULL);
    *flags |= IO_ASYNCH_REPLY;
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
        memo_ctx->memoLength = U2BE(cdata, 0);
        cx_hash((cx_hash_t *) &tx_state->hash, 0, cdata, 2, NULL, 0);

        ctx->state = TX_ENCRYPTED_AMOUNT_TRANSFER_MEMO_START;
        ux_flow_init(0, ux_sign_encrypted_amount_transfer_initial, NULL);
        *flags |= IO_ASYNCH_REPLY;
    } else if (p1 == P1_MEMO && ctx->state == TX_ENCRYPTED_AMOUNT_TRANSFER_MEMO_START) {
        cx_hash((cx_hash_t *) &tx_state->hash, 0, cdata, dataLength, NULL, 0);

        // Read initial part of memo and then display it:
        readMemoInitial(cdata, dataLength);

        if (memo_ctx->memoLength == 0) {
            finishMemoEncrypted(flags);
        } else {
            ctx->state = TX_ENCRYPTED_AMOUNT_TRANSFER_MEMO;
            sendSuccessNoIdle();
        }
    } else if (p1 == P1_MEMO && ctx->state == TX_ENCRYPTED_AMOUNT_TRANSFER_MEMO) {
        cx_hash((cx_hash_t *) &tx_state->hash, 0, cdata, dataLength, NULL, 0);

        // Read current part of memo and then display it:
        readMemoContent(cdata, dataLength);

        if (memo_ctx->memoLength != 0) {
            // The memo size is <=256 bytes, so we should always have received the complete memo by this point;
            THROW(ERROR_INVALID_STATE);
        }

        finishMemoEncrypted(flags);
    } else if (p1 == P1_REMAINING_AMOUNT && ctx->state == TX_ENCRYPTED_AMOUNT_TRANSFER_REMAINING_AMOUNT) {
        handleRemainingAmount(cdata);
    } else if (
        p1 == P1_TRANSFER_AMOUNT_AGG_INDEX_PROOF_SIZE && ctx->state == TX_ENCRYPTED_AMOUNT_TRANSFER_TRANSFER_AMOUNT) {
        handleTransferAmountAggIndexProofSize(cdata);
    } else if (p1 == P1_PROOF && ctx->state == TX_ENCRYPTED_AMOUNT_TRANSFER_PROOFS) {
        handleProofs(cdata, dataLength, flags, ux_sign_flow_shared);
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
        handleProofs(cdata, dataLength, flags, ux_sign_encrypted_amount_transfer);
    } else {
        THROW(ERROR_INVALID_STATE);
    }
}
