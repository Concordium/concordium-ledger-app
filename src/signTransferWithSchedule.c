#include <os.h>
#include <os_io_seproxyhal.h>
#include "cx.h"
#include <stdint.h>
#include "menu.h"
#include "util.h"
#include <string.h>
#include "base58check.h"
#include "sign.h"

static signTransferWithScheduleContext_t *ctx = &global.signTransferWithScheduleContext;
static tx_state_t *tx_state = &global_tx_state;

void processNextScheduledAmount(uint8_t *buffer);

// UI definitions for displaying the transaction contents of the first packet for verification before continuing
// to process the scheduled amount pairs that will be received in separate packets.
UX_STEP_NOCB(
    ux_scheduled_transfer_initial_flow_0_step,
    nn,
    {
      "Review",
      "transaction"
    });
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
    sendSuccessNoIdle(),
    {
      "Continue",
      "with transaction"
    });
UX_FLOW(ux_scheduled_transfer_initial_flow,
    &ux_scheduled_transfer_initial_flow_0_step,
    &ux_scheduled_transfer_initial_flow_1_step,
    &ux_scheduled_transfer_initial_flow_2_step
);

// UI definitions for displaying a timestamp and an amount of a scheduled transfer.
UX_STEP_NOCB(
    ux_sign_scheduled_transfer_pair_flow_0_step,
    bn,
    {
        "Timestamp",
        (char *) global.signTransferWithScheduleContext.displayTimestamp
    });
UX_STEP_NOCB(
    ux_sign_scheduled_transfer_pair_flow_1_step,
    bn,
    {
        "Amount (uGTU)",
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
        // Current packet has been successfully read, but there are still more data to receive. Ask the computer
        // for more data.
        sendSuccessNoIdle();
    } else {
        // The current packet still has additional timestamp/amount pairs to be added to the hash and
        // displayed for the user.
        uint64_t timestamp = U8BE(ctx->buffer, ctx->pos);
        cx_hash((cx_hash_t *) &tx_state->hash, 0, buffer + ctx->pos, 8, NULL, 0);
        ctx->pos += 8;
        bin2dec(ctx->displayTimestamp, timestamp);

        uint64_t amount = U8BE(ctx->buffer, ctx->pos);
        cx_hash((cx_hash_t *) &tx_state->hash, 0, buffer + ctx->pos, 8, NULL, 0);
        ctx->pos += 8;
        bin2dec(ctx->displayAmount, amount);

        // We read one more scheduled amount, so count down to keep track of remaining to process.
        ctx->scheduledAmountsInCurrentPacket -= 1;

        // Display the timestamp and amount for the user to validate it.
        ux_flow_init(0, ux_sign_scheduled_transfer_pair_flow, NULL);
    }
}

// APDU parameters specific to transfer with schedule transaction (multiple packets protocol).
#define P1_INITIAL_PACKET           0x00    // Sent for 1st packet of the transfer.
#define P1_SCHEDULED_TRANSFER_PAIRS 0x01    // Sent for succeeding packets containing (timestamp, amount) pairs.

void handleSignTransferWithSchedule(uint8_t *dataBuffer, uint8_t p1, volatile unsigned int *flags) {
    // Send error back to computer if the received APDU parameter is invalid.
    if (p1 != P1_INITIAL_PACKET && p1 != P1_SCHEDULED_TRANSFER_PAIRS) {
        THROW(0x6B01);
    }

    // Ensure that the received transaction is well-formed, i.e. that we only receive an initial packet once,
    // and it was the first packet received. This presents an attack where two transactions are concatenated.
    if (p1 == P1_INITIAL_PACKET) {
        if (tx_state->initialized) {
            THROW(0x6B01);
        }
        tx_state->initialized = true;
    } else {
        if (!tx_state->initialized) {
            THROW(0x6B01);
        }
    }

    if (p1 == P1_INITIAL_PACKET) {
        int bytesRead = parseKeyDerivationPath(dataBuffer);
        dataBuffer += bytesRead;

        // Initialize the transaction hash object.
        cx_sha256_init(&tx_state->hash);

        // Add transaction header to the hash.
        cx_hash((cx_hash_t *) &tx_state->hash, 0, dataBuffer, 60, NULL, 0);
        dataBuffer += 60;

        // Transaction payload/body comes right after the transaction header. First byte determines the transaction kind.
        uint8_t transactionKind[1];
        os_memmove(transactionKind, dataBuffer, 1);
        dataBuffer += 1;
        cx_hash((cx_hash_t *) &tx_state->hash, 0, transactionKind, 1, NULL, 0);

        // Extract the destination address and add to hash.
        uint8_t toAddress[32];
        os_memmove(toAddress, dataBuffer, 32);
        dataBuffer += 32;
        cx_hash((cx_hash_t *) &tx_state->hash, 0, toAddress, 32, NULL, 0);

        // Used in display of recipient address.
        size_t outputSize = sizeof(ctx->displayStr);
        if (base58check_encode(toAddress, sizeof(toAddress), ctx->displayStr, &outputSize) != 0) {
            THROW(0x6B01);  // The received address bytes are not valid a valid base58 encoding.
        }
        ctx->displayStr[50] = '\0';

        uint8_t numberOfScheduledAmountsArray[1];
        os_memmove(numberOfScheduledAmountsArray, dataBuffer, 1);
        ctx->remainingNumberOfScheduledAmounts = numberOfScheduledAmountsArray[0];
        cx_hash((cx_hash_t *) &tx_state->hash, 0, numberOfScheduledAmountsArray, 1, NULL, 0);
        dataBuffer += 1;

        // Display the transaction information to the user (recipient address and amount to be sent).
        ux_flow_init(0, ux_scheduled_transfer_initial_flow, NULL);

        // Tell the main process to wait for a button press.
        *flags |= IO_ASYNCH_REPLY;
    } else {
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

        os_memmove(ctx->buffer, dataBuffer, ctx->scheduledAmountsInCurrentPacket * 16);
        processNextScheduledAmount(ctx->buffer);

        // Tell the main process to wait for a button press.
        *flags |= IO_ASYNCH_REPLY;
    }
}