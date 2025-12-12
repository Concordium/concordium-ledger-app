#include "globals.h"

static signTransferContext_t *ctx = &global.withDataBlob.signTransferContext;
static cborContext_t *memo_ctx = &global.withDataBlob.cborContext;
static tx_state_t *tx_state = &global_tx_state;

#define P1_INITIAL           0x00
#define P1_INITIAL_WITH_MEMO 0x01
#define P1_MEMO              0x02
#define P1_AMOUNT            0x03

void handleSignTransfer(uint8_t *cdata, uint8_t lc, volatile unsigned int *flags) {
    uint8_t offset =
        handleHeaderAndToAddress(cdata, lc, TRANSFER, ctx->displayStr, sizeof(ctx->displayStr));
    cdata += offset;
    uint8_t remainingDataLength = lc - offset;

    // Build display value of the amount to transfer, and also add the bytes to the hash.
    if (remainingDataLength < 8) {
        THROW(ERROR_BUFFER_OVERFLOW);
    }
    uint64_t amount = U8BE(cdata, 0);
    amountToGtuDisplay(ctx->displayAmount, sizeof(ctx->displayAmount), amount);
    updateHash((cx_hash_t *)&tx_state->hash, cdata, 8);

    // Display the transaction information to the user (recipient address and amount to be sent).
    startTransferDisplay(false, flags);

    // Tell the main process to wait for a button press.
    *flags |= IO_ASYNCH_REPLY;
}

void finishMemo() {
    ctx->state = TX_TRANSFER_AMOUNT;
    sendSuccessNoIdle();
}

void handleSignTransferWithMemo(uint8_t *cdata,
                                uint8_t p1,
                                uint8_t dataLength,
                                volatile unsigned int *flags,
                                bool isInitialCall) {
    if (isInitialCall) {
        ctx->state = TX_TRANSFER_INITIAL;
    }
    uint8_t remainingDataLength = dataLength;
    if (p1 == P1_INITIAL_WITH_MEMO && ctx->state == TX_TRANSFER_INITIAL) {
        uint8_t offset = handleHeaderAndToAddress(cdata,
                                                  remainingDataLength,
                                                  TRANSFER_WITH_MEMO,
                                                  ctx->displayStr,
                                                  sizeof(ctx->displayStr));
        cdata += offset;
        remainingDataLength -= offset;
        // hash the memo length
        if (remainingDataLength < 2) {
            THROW(ERROR_BUFFER_OVERFLOW);
        }
        memo_ctx->cborLength = U2BE(cdata, 0);
        if (memo_ctx->cborLength > MAX_MEMO_SIZE) {
            THROW(ERROR_INVALID_PARAM);
        }

        updateHash((cx_hash_t *)&tx_state->hash, cdata, 2);

        ctx->state = TX_TRANSFER_MEMO_INITIAL;
        sendSuccessNoIdle();
    } else if (p1 == P1_MEMO && ctx->state == TX_TRANSFER_MEMO_INITIAL) {
        updateHash((cx_hash_t *)&tx_state->hash, cdata, dataLength);

        readCborInitial(cdata, dataLength);
        if (memo_ctx->cborLength == 0) {
            finishMemo();
        } else {
            ctx->state = TX_TRANSFER_MEMO;
            sendSuccessNoIdle();
        }
    } else if (p1 == P1_MEMO && ctx->state == TX_TRANSFER_MEMO) {
        updateHash((cx_hash_t *)&tx_state->hash, cdata, dataLength);

        readCborContent(cdata, dataLength);
        if (memo_ctx->cborLength != 0) {
            // The memo size is <=256 bytes, so we should always have received the complete memo by
            // this point
            THROW(ERROR_INVALID_STATE);
        }

        finishMemo();
    } else if (p1 == P1_AMOUNT && ctx->state == TX_TRANSFER_AMOUNT) {
        // Build display value of the amount to transfer, and also add the bytes to the hash.
        if (remainingDataLength < 8) {
            THROW(ERROR_BUFFER_OVERFLOW);
        }
        uint64_t amount = U8BE(cdata, 0);
        amountToGtuDisplay(ctx->displayAmount, sizeof(ctx->displayAmount), amount);
        updateHash((cx_hash_t *)&tx_state->hash, cdata, 8);

        startTransferDisplay(true, flags);

    } else {
        THROW(ERROR_INVALID_STATE);
    }
}
