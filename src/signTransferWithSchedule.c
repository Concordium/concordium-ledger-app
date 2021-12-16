#include <os.h>

#include "accountSenderView.h"
#include "base58check.h"
#include "responseCodes.h"
#include "sign.h"
#include "time.h"
#include "util.h"

static signTransferWithScheduleContext_t *ctx = &global.withDataBlob.signTransferWithScheduleContext;
static CborContext_t *memo_ctx = &global.withDataBlob.cborContext;
static tx_state_t *tx_state = &global_tx_state;

void processNextScheduledAmount(uint8_t *buffer);

// UI definitions for displaying the transaction contents of the first packet for verification before continuing
// to process the scheduled amount pairs that will be received in separate packets.
UX_STEP_NOCB(
    ux_scheduled_transfer_initial_flow_1_step,
    bnnn_paging,
    {.title = "Recipient", .text = (char *) global.withDataBlob.signTransferWithScheduleContext.displayStr});
UX_STEP_VALID(ux_scheduled_transfer_initial_flow_2_step, nn, sendSuccessNoIdle(), {"Continue", "with transaction"});
UX_FLOW(
    ux_scheduled_transfer_initial_flow,
    &ux_sign_flow_shared_review,
    &ux_sign_flow_account_sender_view,
    &ux_scheduled_transfer_initial_flow_1_step,
    &ux_scheduled_transfer_initial_flow_2_step);

// UI definitions for displaying a timestamp and an amount of a scheduled transfer.
UX_STEP_NOCB(
    ux_sign_scheduled_transfer_pair_flow_0_step,
    bnnn_paging,
    {"Release time (UTC)", (char *) global.withDataBlob.signTransferWithScheduleContext.displayTimestamp});
UX_STEP_NOCB(
    ux_sign_scheduled_transfer_pair_flow_1_step,
    bnnn_paging,
    {"Amount", (char *) global.withDataBlob.signTransferWithScheduleContext.displayAmount});
UX_STEP_CB(
    ux_sign_scheduled_transfer_pair_flow_2_step,
    nn,
    processNextScheduledAmount(ctx->buffer),
    {"Continue", "with transaction"});
UX_FLOW(
    ux_sign_scheduled_transfer_pair_flow,
    &ux_sign_scheduled_transfer_pair_flow_0_step,
    &ux_sign_scheduled_transfer_pair_flow_1_step,
    &ux_sign_scheduled_transfer_pair_flow_2_step);

void processNextScheduledAmount(uint8_t *buffer) {
    // The full transaction has been added to the hash, so we can continue to the signing process.
    if (ctx->remainingNumberOfScheduledAmounts == 0 && ctx->scheduledAmountsInCurrentPacket == 0) {
        ux_flow_init(0, ux_sign_flow_shared, NULL);
    } else if (ctx->scheduledAmountsInCurrentPacket == 0) {
        // Current packet has been successfully read, but there are still more data to receive. Ask the caller
        // for more data.
        sendSuccessNoIdle();
    } else {
        // The current packet still has additional timestamp/amount pairs to be added to the hash and
        // displayed for the user.
        uint64_t timestamp = U8BE(ctx->buffer, ctx->pos) / 1000;
        cx_hash((cx_hash_t *) &tx_state->hash, 0, buffer + ctx->pos, 8, NULL, 0);
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

        uint64_t amount = U8BE(ctx->buffer, ctx->pos);
        cx_hash((cx_hash_t *) &tx_state->hash, 0, buffer + ctx->pos, 8, NULL, 0);
        ctx->pos += 8;
        amountToGtuDisplay(ctx->displayAmount, sizeof(ctx->displayAmount), amount);

        // We read one more scheduled amount, so count down to keep track of remaining to process.
        ctx->scheduledAmountsInCurrentPacket -= 1;

        // Display the timestamp and amount for the user to validate it.
        ux_flow_init(0, ux_sign_scheduled_transfer_pair_flow, NULL);
    }
}

void handleTransferPairs(uint8_t *cdata, volatile unsigned int *flags) {
    // Load the scheduled transfer information.
    // First 8 bytes is the timestamp, the following 8 bytes is the amount.
    // We have room for 255 bytes, so 240 = 15 * 16, i.e. 15 pairs in each packet. Determine how many pairs are
    // in the current packet.
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
    cx_hash((cx_hash_t *) &tx_state->hash, 0, &ctx->remainingNumberOfScheduledAmounts, 1, NULL, 0);
    ctx->state = TX_TRANSFER_WITH_SCHEDULE_TRANSFER_PAIRS;
    ux_flow_init(0, ux_display_cbor, NULL);
    *flags |= IO_ASYNCH_REPLY;
}

void handleSignTransferWithScheduleAndMemo(
    uint8_t *cdata,
    uint8_t p1,
    uint8_t dataLength,
    volatile unsigned int *flags,
    bool isInitialCall) {
    if (isInitialCall) {
        ctx->state = TX_TRANSFER_WITH_SCHEDULE_INITIAL;
    }

    if (p1 == P1_INITIAL_WITH_MEMO && ctx->state == TX_TRANSFER_WITH_SCHEDULE_INITIAL) {
        cdata +=
            handleHeaderAndToAddress(cdata, TRANSFER_WITH_SCHEDULE_WITH_MEMO, ctx->displayStr, sizeof(ctx->displayStr));

        // Store the number of scheduled amounts we are going to receive next.
        ctx->remainingNumberOfScheduledAmounts = cdata[0];
        cdata += 1;

        // Hash memo length
        memo_ctx->cborLength = U2BE(cdata, 0);
        if (memo_ctx->cborLength > MAX_MEMO_SIZE) {
            THROW(ERROR_INVALID_PARAM);
        }

        cx_hash((cx_hash_t *) &tx_state->hash, 0, cdata, 2, NULL, 0);

        // Update the state to expect the next message to contain the first bytes of the memo.
        ctx->state = TX_TRANSFER_WITH_SCHEDULE_MEMO_START;
        // Display the transaction information to the user (recipient address and amount to be sent).
        ux_flow_init(0, ux_scheduled_transfer_initial_flow, NULL);

        *flags |= IO_ASYNCH_REPLY;
    } else if (p1 == P1_MEMO && ctx->state == TX_TRANSFER_WITH_SCHEDULE_MEMO_START) {
        cx_hash((cx_hash_t *) &tx_state->hash, 0, cdata, dataLength, NULL, 0);

        // Read initial part of memo and then display it:
        readCborInitial(cdata, dataLength);

        if (memo_ctx->cborLength == 0) {
            finishMemoScheduled(flags);
        } else {
            ctx->state = TX_TRANSFER_WITH_SCHEDULE_MEMO;
            sendSuccessNoIdle();
        }

    } else if (p1 == P1_MEMO && ctx->state == TX_TRANSFER_WITH_SCHEDULE_MEMO) {
        cx_hash((cx_hash_t *) &tx_state->hash, 0, cdata, dataLength, NULL, 0);

        // Read current part of memo and then display it:
        readCborContent(cdata, dataLength);

        if (memo_ctx->cborLength != 0) {
            // The memo size is <=256 bytes, so we should always have received the complete memo by this point;
            THROW(ERROR_INVALID_STATE);
        }

        finishMemoScheduled(flags);
    } else if (p1 == P1_SCHEDULED_TRANSFER_PAIRS && ctx->state == TX_TRANSFER_WITH_SCHEDULE_TRANSFER_PAIRS) {
        handleTransferPairs(cdata, flags);
    } else {
        THROW(ERROR_INVALID_PARAM);
    }
}

void handleSignTransferWithSchedule(uint8_t *cdata, uint8_t p1, volatile unsigned int *flags, bool isInitialCall) {
    if (isInitialCall) {
        ctx->state = TX_TRANSFER_WITH_SCHEDULE_INITIAL;
    }

    if (p1 == P1_INITIAL_PACKET && ctx->state == TX_TRANSFER_WITH_SCHEDULE_INITIAL) {
        cdata += handleHeaderAndToAddress(cdata, TRANSFER_WITH_SCHEDULE, ctx->displayStr, sizeof(ctx->displayStr));

        // Store the number of scheduled amounts we are going to receive next.
        ctx->remainingNumberOfScheduledAmounts = cdata[0];
        // Hash schedule length
        cx_hash((cx_hash_t *) &tx_state->hash, 0, cdata, 1, NULL, 0);

        // Update the state to expect the next message to contain transfer pairs.
        ctx->state = TX_TRANSFER_WITH_SCHEDULE_TRANSFER_PAIRS;
        // Display the transaction information to the user (recipient address and amount to be sent).
        ux_flow_init(0, ux_scheduled_transfer_initial_flow, NULL);

        *flags |= IO_ASYNCH_REPLY;
    } else if (p1 == P1_SCHEDULED_TRANSFER_PAIRS && ctx->state == TX_TRANSFER_WITH_SCHEDULE_TRANSFER_PAIRS) {
        handleTransferPairs(cdata, flags);
    } else {
        THROW(ERROR_INVALID_PARAM);
    }
}
