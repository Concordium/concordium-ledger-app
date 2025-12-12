#include "globals.h"

static signTransferWithScheduleContext_t *ctx =
    &global.withDataBlob.signTransferWithScheduleContext;
static cborContext_t *memo_ctx = &global.withDataBlob.cborContext;
static tx_state_t *tx_state = &global_tx_state;

void processNextScheduledAmount(uint8_t *buffer) {
    LEDGER_ASSERT(buffer != NULL, "NULL buffer");
    if (ctx->scheduledAmountsInCurrentPacket == 0) {
        // Current packet has been successfully read, but there are still more data to receive. Ask
        // the caller for more data.
        sendSuccessNoIdle();
    } else {
        // The current packet still has additional timestamp/amount pairs to be added to the hash
        // and displayed for the user.
        if (ctx->pos + 8 > sizeof(ctx->buffer)) {
            THROW(ERROR_BUFFER_OVERFLOW);
        }
        uint64_t timestamp = U8BE(ctx->buffer, ctx->pos) / 1000;
        updateHash((cx_hash_t *)&tx_state->hash, buffer + ctx->pos, 8);
        ctx->pos += 8;
        int valid = secondsToTm(timestamp, &ctx->time);
        if (valid != 0) {
            THROW(ERROR_INVALID_PARAM);
        }

        // If the year is too far into the future, then just fail. This is needed so
        // that we know how much space to reserve to display the date time.
        if (ctx->time.tm_year > 9999) {
            THROW(ERROR_INVALID_PARAM);
        }
        timeToDisplayText(ctx->time, ctx->displayTimestamp, sizeof(ctx->displayTimestamp));
        if (ctx->pos + 8 > sizeof(ctx->buffer)) {
            THROW(ERROR_BUFFER_OVERFLOW);
        }
        uint64_t amount = U8BE(ctx->buffer, ctx->pos);
        updateHash((cx_hash_t *)&tx_state->hash, buffer + ctx->pos, 8);
        ctx->pos += 8;
        amountToGtuDisplay(ctx->displayAmount, sizeof(ctx->displayAmount), amount);

        // We read one more scheduled amount, so count down to keep track of remaining to process.
        ctx->scheduledAmountsInCurrentPacket -= 1;

        // If it is the final schedule pair, then also allow the user to sign or decline the
        // transaction.
        if (ctx->remainingNumberOfScheduledAmounts == 0 &&
            ctx->scheduledAmountsInCurrentPacket == 0) {
            uiSignScheduledTransferPairFlowSignDisplay();
        } else {
            // Display the timestamp and amount for the user to validate it.
            uiSignScheduledTransferPairFlowDisplay();
        }
    }
}

void handleTransferPairs(uint8_t *cdata, uint8_t dataLength, volatile unsigned int *flags) {
    // Load the scheduled transfer information.
    // First 8 bytes is the timestamp, the following 8 bytes is the amount.
    // We have room for 255 bytes, so 240 = 15 * 16, i.e. 15 pairs in each packet. Determine how
    // many pairs are in the current packet.
    if (ctx->remainingNumberOfScheduledAmounts <= 15) {
        ctx->scheduledAmountsInCurrentPacket = ctx->remainingNumberOfScheduledAmounts;
        ctx->remainingNumberOfScheduledAmounts = 0;
    } else {
        // The maximum is available in the packet.
        ctx->scheduledAmountsInCurrentPacket = 15;
        ctx->remainingNumberOfScheduledAmounts -= 15;
    }

    // Reset pointer keeping track of where we are in the current packet being processed.
    ctx->pos = 0;

    if (ctx->scheduledAmountsInCurrentPacket * 16 > sizeof(ctx->buffer) ||
        dataLength < ctx->scheduledAmountsInCurrentPacket * 16) {
        THROW(ERROR_BUFFER_OVERFLOW);
    }
    memmove(ctx->buffer, cdata, ctx->scheduledAmountsInCurrentPacket * 16);
    processNextScheduledAmount(ctx->buffer);

    // Tell the main process to wait for a button press.
    *flags |= IO_ASYNCH_REPLY;
}

#define P1_INITIAL_PACKET           0x00
#define P1_SCHEDULED_TRANSFER_PAIRS 0x01
#define P1_INITIAL_WITH_MEMO        0x02
#define P1_MEMO                     0x03

void finishMemoScheduled(volatile unsigned int *flags) {
    updateHash((cx_hash_t *)&tx_state->hash, &ctx->remainingNumberOfScheduledAmounts, 1);
    ctx->state = TX_TRANSFER_WITH_SCHEDULE_TRANSFER_PAIRS;
    startInitialScheduledTransferDisplay(true);
    *flags |= IO_ASYNCH_REPLY;
}

void handleSignTransferWithScheduleAndMemo(uint8_t *cdata,
                                           uint8_t p1,
                                           uint8_t dataLength,
                                           volatile unsigned int *flags,
                                           bool isInitialCall) {
    if (isInitialCall) {
        ctx->state = TX_TRANSFER_WITH_SCHEDULE_INITIAL;
    }

    if (p1 == P1_INITIAL_WITH_MEMO && ctx->state == TX_TRANSFER_WITH_SCHEDULE_INITIAL) {
        cdata += handleHeaderAndToAddress(cdata,
                                          dataLength,
                                          TRANSFER_WITH_SCHEDULE_WITH_MEMO,
                                          ctx->displayStr,
                                          sizeof(ctx->displayStr));

        // Store the number of scheduled amounts we are going to receive next.
        if (dataLength < 1) {
            THROW(ERROR_BUFFER_OVERFLOW);
        }
        ctx->remainingNumberOfScheduledAmounts = cdata[0];
        cdata += 1;

        // Hash memo length
        if (dataLength < 2) {
            THROW(ERROR_BUFFER_OVERFLOW);
        }
        memo_ctx->cborLength = U2BE(cdata, 0);
        if (memo_ctx->cborLength > MAX_MEMO_SIZE) {
            THROW(ERROR_INVALID_PARAM);
        }

        updateHash((cx_hash_t *)&tx_state->hash, cdata, 2);

        // Update the state to expect the next message to contain the first bytes of the memo.
        ctx->state = TX_TRANSFER_WITH_SCHEDULE_MEMO_START;
        sendSuccessNoIdle();
    } else if (p1 == P1_MEMO && ctx->state == TX_TRANSFER_WITH_SCHEDULE_MEMO_START) {
        updateHash((cx_hash_t *)&tx_state->hash, cdata, dataLength);

        // Read initial part of memo and then display it:
        readCborInitial(cdata, dataLength);

        if (memo_ctx->cborLength == 0) {
            finishMemoScheduled(flags);
        } else {
            ctx->state = TX_TRANSFER_WITH_SCHEDULE_MEMO;
            sendSuccessNoIdle();
        }
    } else if (p1 == P1_MEMO && ctx->state == TX_TRANSFER_WITH_SCHEDULE_MEMO) {
        updateHash((cx_hash_t *)&tx_state->hash, cdata, dataLength);

        // Read current part of memo and then display it:
        readCborContent(cdata, dataLength);

        if (memo_ctx->cborLength != 0) {
            // The memo size is <=256 bytes, so we should always have received the complete memo by
            // this point;
            THROW(ERROR_INVALID_STATE);
        }

        finishMemoScheduled(flags);
    } else if (p1 == P1_SCHEDULED_TRANSFER_PAIRS &&
               ctx->state == TX_TRANSFER_WITH_SCHEDULE_TRANSFER_PAIRS) {
        handleTransferPairs(cdata, dataLength, flags);
    } else {
        THROW(ERROR_INVALID_PARAM);
    }
}

void handleSignTransferWithSchedule(uint8_t *cdata,
                                    uint8_t p1,
                                    uint8_t lc,
                                    volatile unsigned int *flags,
                                    bool isInitialCall) {
    if (isInitialCall) {
        ctx->state = TX_TRANSFER_WITH_SCHEDULE_INITIAL;
    }

    if (p1 == P1_INITIAL_PACKET && ctx->state == TX_TRANSFER_WITH_SCHEDULE_INITIAL) {
        cdata += handleHeaderAndToAddress(cdata,
                                          lc,
                                          TRANSFER_WITH_SCHEDULE,
                                          ctx->displayStr,
                                          sizeof(ctx->displayStr));

        // Store the number of scheduled amounts we are going to receive next.
        if (lc < 1) {
            THROW(ERROR_BUFFER_OVERFLOW);
        }
        ctx->remainingNumberOfScheduledAmounts = cdata[0];
        // Hash schedule length
        updateHash((cx_hash_t *)&tx_state->hash, cdata, 1);

        // Update the state to expect the next message to contain transfer pairs.
        ctx->state = TX_TRANSFER_WITH_SCHEDULE_TRANSFER_PAIRS;
        // Display the transaction information to the user (recipient address and amount to be
        // sent).
        startInitialScheduledTransferDisplay(false);
        *flags |= IO_ASYNCH_REPLY;
    } else if (p1 == P1_SCHEDULED_TRANSFER_PAIRS &&
               ctx->state == TX_TRANSFER_WITH_SCHEDULE_TRANSFER_PAIRS) {
        handleTransferPairs(cdata, lc, flags);
    } else {
        THROW(ERROR_INVALID_PARAM);
    }
}
