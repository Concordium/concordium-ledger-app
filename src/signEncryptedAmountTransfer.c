#include <os.h>
#include "util.h"
#include "accountSenderView.h"
#include "memo.h"
#include "sign.h"
#include "base58check.h"
#include "responseCodes.h"

static signEncryptedAmountToTransfer_t *ctx = &global.withMemo.signEncryptedAmountToTransfer;
static memoContext_t *memo_ctx = &global.withMemo.memoContext;
static tx_state_t *tx_state = &global_tx_state;

// UI for displaying encrypted transfer transaction. It only shows the user the recipient address
// as the amounts are encrypted and can't be validated by the user.
UX_STEP_NOCB(
    ux_sign_encrypted_amount_transfer_1_step,
    nn,
    {
        "Shielded",
        "transfer"
    });
UX_STEP_CB(
    ux_sign_encrypted_amount_transfer_2_step,
    bnnn_paging,
    sendSuccessNoIdle(),
    {
        .title = "Recipient",
        .text = (char *) global.withMemo.signEncryptedAmountToTransfer.to
    });
UX_FLOW(ux_sign_encrypted_amount_transfer,
    &ux_sign_flow_shared_review,
    &ux_sign_encrypted_amount_transfer_1_step,
    &ux_sign_flow_account_sender_view,
    &ux_sign_encrypted_amount_transfer_2_step,
    &ux_sign_flow_shared_sign,
    &ux_sign_flow_shared_decline
);

UX_FLOW(ux_sign_encrypted_amount_transfer_initial,
    &ux_sign_flow_shared_review,
    &ux_sign_encrypted_amount_transfer_1_step,
    &ux_sign_flow_account_sender_view,
    &ux_sign_encrypted_amount_transfer_2_step
);

#define P1_INITIAL                              0x00
#define P1_REMAINING_AMOUNT                     0x01
#define P1_TRANSFER_AMOUNT_AGG_INDEX_PROOF_SIZE 0x02
#define P1_PROOF                                0x03
#define P1_INITIAL_WITH_MEMO                    0x04
#define P1_MEMO                                 0x05

int handleHeaderAndToAddressEncryptedTransfer(uint8_t *cdata, uint8_t kind) {
    // Parse the key derivation path, which should always be the first thing received
    // in a command to the Ledger application.
    int keyPathLength = parseKeyDerivationPath(cdata);
    cdata += keyPathLength;

    // Initialize the hash that will be the hash of the whole transaction, which is what will be signed
    // if the user approves.
    cx_sha256_init(&tx_state->hash);
    int headerLength = hashAccountTransactionHeaderAndKind(cdata, kind);
    cdata += headerLength;

    // Extract the recipient address and add to the hash.
    uint8_t toAddress[32];
    memmove(toAddress, cdata, 32);
    cdata += 32;
    cx_hash((cx_hash_t *) &tx_state->hash, 0, toAddress, 32, NULL, 0);

    // The recipient address is in a base58 format, so we need to encode it to be
    // able to display in a humand-readable way. This is written to ctx->displayStr as a string
    // so that it can be displayed.
    size_t outputSize = sizeof(ctx->to);
    if (base58check_encode(toAddress, sizeof(toAddress), ctx->to, &outputSize) != 0) {
        // The received address bytes are not a valid base58 encoding.
        THROW(ERROR_INVALID_TRANSACTION);
    }
    ctx->to[55] = '\0';

    return keyPathLength + headerLength + 32;
}

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

void handleProofs(uint8_t *cdata, uint8_t dataLength, volatile unsigned int *flags,  const ux_flow_step_t* const *finishedFlow) {
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

void handleSignEncryptedAmountTransferWithMemo(uint8_t *cdata, uint8_t p1, uint8_t dataLength, volatile unsigned int *flags, bool isInitialCall) {
    if (isInitialCall) {
        ctx->state = TX_ENCRYPTED_AMOUNT_TRANSFER_INITIAL;
    }

    if (memo_ctx->memoLength == 0 && ctx->state == TX_ENCRYPTED_AMOUNT_TRANSFER_MEMO) {
        ctx->state = TX_ENCRYPTED_AMOUNT_TRANSFER_REMAINING_AMOUNT;
    }

    if (p1 == P1_INITIAL_WITH_MEMO && ctx->state == TX_ENCRYPTED_AMOUNT_TRANSFER_INITIAL) {
        cdata += handleHeaderAndToAddressEncryptedTransfer(cdata, ENCRYPTED_AMOUNT_TRANSFER_WITH_MEMO);

        // Hash memo length
        memo_ctx->memoLength = U2BE(cdata, 0);
        cx_hash((cx_hash_t *) &tx_state->hash, 0, cdata, 2, NULL, 0);
        cdata += 2;

        ctx->state = TX_ENCRYPTED_AMOUNT_TRANSFER_MEMO_START;
        ux_flow_init(0, ux_sign_encrypted_amount_transfer_initial, NULL);
        *flags |= IO_ASYNCH_REPLY;
    } else if (p1 == P1_MEMO && ctx->state == TX_ENCRYPTED_AMOUNT_TRANSFER_MEMO_START) {
        cx_hash((cx_hash_t *) &tx_state->hash, 0, cdata, dataLength, NULL, 0);

        // Read initial part of memo and then display it:
        readMemoInitial(cdata, dataLength);
        ctx->state = TX_ENCRYPTED_AMOUNT_TRANSFER_MEMO;
        ux_flow_init(0, ux_sign_transfer_memo, NULL);
        *flags |= IO_ASYNCH_REPLY;
    } else if (p1 == P1_MEMO && ctx->state == TX_ENCRYPTED_AMOUNT_TRANSFER_MEMO) {
        cx_hash((cx_hash_t *) &tx_state->hash, 0, cdata, dataLength, NULL, 0);

        // Read current part of memo and then display it:
        readMemoContent(cdata, dataLength);
        ux_flow_init(0, ux_sign_transfer_memo, NULL);
        *flags |= IO_ASYNCH_REPLY;
    } else if (p1 == P1_REMAINING_AMOUNT && ctx->state == TX_ENCRYPTED_AMOUNT_TRANSFER_REMAINING_AMOUNT) {
        handleRemainingAmount(cdata);
    } else if (p1 == P1_TRANSFER_AMOUNT_AGG_INDEX_PROOF_SIZE && ctx->state == TX_ENCRYPTED_AMOUNT_TRANSFER_TRANSFER_AMOUNT) {
        handleTransferAmountAggIndexProofSize(cdata);
    } else if (p1 == P1_PROOF && ctx->state == TX_ENCRYPTED_AMOUNT_TRANSFER_PROOFS) {
        handleProofs(cdata, dataLength, flags, ux_sign_flow_shared);
    } else {
        THROW(ERROR_INVALID_STATE);
    }
}

void handleSignEncryptedAmountTransfer(uint8_t *cdata, uint8_t p1, uint8_t dataLength, volatile unsigned int *flags, bool isInitialCall) {
    if (isInitialCall) {
        ctx->state = TX_ENCRYPTED_AMOUNT_TRANSFER_INITIAL;
    }
    if (p1 == P1_INITIAL && ctx->state == TX_ENCRYPTED_AMOUNT_TRANSFER_INITIAL) {
        cdata += handleHeaderAndToAddressEncryptedTransfer(cdata, ENCRYPTED_AMOUNT_TRANSFER);
        ctx->state = TX_ENCRYPTED_AMOUNT_TRANSFER_REMAINING_AMOUNT;
        sendSuccessNoIdle();
    } else if (p1 == P1_REMAINING_AMOUNT && ctx->state == TX_ENCRYPTED_AMOUNT_TRANSFER_REMAINING_AMOUNT) {
        handleRemainingAmount(cdata);
    } else if (p1 == P1_TRANSFER_AMOUNT_AGG_INDEX_PROOF_SIZE && ctx->state == TX_ENCRYPTED_AMOUNT_TRANSFER_TRANSFER_AMOUNT) {
        handleTransferAmountAggIndexProofSize(cdata);
    } else if (p1 == P1_PROOF && ctx->state == TX_ENCRYPTED_AMOUNT_TRANSFER_PROOFS) {
        handleProofs(cdata, dataLength, flags, ux_sign_encrypted_amount_transfer);
    } else {
        THROW(ERROR_INVALID_STATE);
    }
}
