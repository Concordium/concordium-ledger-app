#include <os.h>
#include "base58check.h"
#include "util.h"
#include "sign.h"
#include "accountSenderView.h"
#include "time.h"
#include "responseCodes.h"

static signTransferWithScheduleContext_t *ctx = &global.signTransferWithScheduleContext;
static tx_state_t *tx_state = &global_tx_state;

void processNextScheduledAmount(uint8_t *buffer);

void updateStateAndSendSuccess() {
    ctx->state = TX_TRANSFER_WITH_SCHEDULE_TRANSFER_PAIRS;
    sendSuccessNoIdle();
}

// UI definitions for displaying the transaction contents of the first packet for verification before continuing
// to process the scheduled amount pairs that will be received in separate packets.
UX_STEP_NOCB(
    ux_scheduled_transfer_initial_flow_1_step,
    bnnn_paging,
    {
      .title = "Recipient",
      .text = (char *) global.signTransferWithScheduleContext.displayStr
    });
UX_STEP_VALID(
    ux_scheduled_transfer_initial_flow_2_step,
    nn,
    updateStateAndSendSuccess(),
    {
      "Continue",
      "with transaction"
    });
UX_FLOW(ux_scheduled_transfer_initial_flow,
    &ux_sign_flow_shared_review,
    &ux_sign_flow_account_sender_view,
    &ux_scheduled_transfer_initial_flow_1_step,
    &ux_scheduled_transfer_initial_flow_2_step
);

// UI definitions for displaying a timestamp and an amount of a scheduled transfer.
UX_STEP_NOCB(
    ux_sign_scheduled_transfer_pair_flow_0_step,
    bnnn_paging,
    {
        "Release time (UTC)",
        (char *) global.signTransferWithScheduleContext.displayTimestamp
    });
UX_STEP_NOCB(
    ux_sign_scheduled_transfer_pair_flow_1_step,
    bnnn_paging,
    {
        "Amount",
        (char *) global.signTransferWithScheduleContext.displayAmount
    });
UX_STEP_CB(
    ux_sign_scheduled_transfer_pair_flow_2_step,
    nn,
    processNextScheduledAmount(ctx->buffer),
    {
      "Continue",
      "with transaction"
    });
UX_FLOW(ux_sign_scheduled_transfer_pair_flow,
    &ux_sign_scheduled_transfer_pair_flow_0_step,
    &ux_sign_scheduled_transfer_pair_flow_1_step,
    &ux_sign_scheduled_transfer_pair_flow_2_step
);

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
        timeToDisplayText(ctx->time, ctx->displayTimestamp);
        
        uint64_t amount = U8BE(ctx->buffer, ctx->pos);
        cx_hash((cx_hash_t *) &tx_state->hash, 0, buffer + ctx->pos, 8, NULL, 0);
        ctx->pos += 8;
        amountToGtuDisplay(ctx->displayAmount, amount);

        // We read one more scheduled amount, so count down to keep track of remaining to process.
        ctx->scheduledAmountsInCurrentPacket -= 1;

        // Display the timestamp and amount for the user to validate it.
        ux_flow_init(0, ux_sign_scheduled_transfer_pair_flow, NULL);
    }
}

#define P1_INITIAL_PACKET           0x00
#define P1_SCHEDULED_TRANSFER_PAIRS 0x01

void handleSignTransferWithSchedule(uint8_t *cdata, uint8_t p1, volatile unsigned int *flags, bool isInitialCall) {
    if (isInitialCall) {
        ctx->state = TX_TRANSFER_WITH_SCHEDULE_INITIAL;
    }

    if (p1 == P1_INITIAL_PACKET && ctx->state == TX_TRANSFER_WITH_SCHEDULE_INITIAL) {
        int bytesRead = parseKeyDerivationPath(cdata);
        cdata += bytesRead;

        // Initialize the transaction hash object.
        cx_sha256_init(&tx_state->hash);
        cdata += hashAccountTransactionHeaderAndKind(cdata, TRANSFER_WITH_SCHEDULE);

        // Extract the destination address and add to hash.
        uint8_t toAddress[32];
        memmove(toAddress, cdata, 32);
        cdata += 32;
        cx_hash((cx_hash_t *) &tx_state->hash, 0, toAddress, 32, NULL, 0);

        // Used in display of recipient address.
        size_t outputSize = sizeof(ctx->displayStr);
        if (base58check_encode(toAddress, sizeof(toAddress), ctx->displayStr, &outputSize) != 0) {
            THROW(ERROR_INVALID_TRANSACTION);
        }
        ctx->displayStr[55] = '\0';

        // Store the number of scheduled amounts we are going to receive next.
        ctx->remainingNumberOfScheduledAmounts = cdata[0];
        cx_hash((cx_hash_t *) &tx_state->hash, 0, cdata, 1, NULL, 0);
        cdata += 1;

        // Display the transaction information to the user (recipient address and amount to be sent).
        ux_flow_init(0, ux_scheduled_transfer_initial_flow, NULL);

        // Tell the main process to wait for a button press.
        *flags |= IO_ASYNCH_REPLY;
    } else if (p1 == P1_SCHEDULED_TRANSFER_PAIRS && ctx->state == TX_TRANSFER_WITH_SCHEDULE_TRANSFER_PAIRS) {
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
    } else {
        THROW(ERROR_INVALID_PARAM);
    }
}
