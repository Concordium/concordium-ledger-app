#include <os.h>
#include <os_io_seproxyhal.h>
#include "cx.h"
#include <stdint.h>
#include "menu.h"
#include "util.h"
#include <string.h>

static accountSubtreePath_t *keyPath = &path;

static char displayStr[65];
static uint8_t displayAccount[8];
static uint8_t numberOfScheduledAmounts;
static uint8_t scheduledAmountsInCurrentPacket;

static uint8_t displayAmount[25];
static uint8_t displayTimestamp[25];

// Buffer to hold the incoming databuffer so that we can iterate over it.
static uint8_t buffer[255];
static uint8_t pos;

void nextPair(uint8_t *buffer);

// UI definitions for displaying a timestamp and an amount of a scheduled transfer.
UX_STEP_NOCB(
    ux_sign_scheduled_transfer_pair_flow_0_step,
    bn,
    {
        "Timestamp",
        (char *) displayTimestamp
    });
UX_STEP_NOCB(
    ux_sign_scheduled_transfer_pair_flow_1_step,
    bn,
    {
        "Amount",
        (char *) displayAmount
    });
UX_STEP_CB(
    ux_sign_scheduled_transfer_pair_flow_2_step,
    nn,
    nextPair(buffer),
    {
      "Continue",
      "with transaction"
    });
UX_FLOW(ux_sign_scheduled_transfer_pair_flow,
    &ux_sign_scheduled_transfer_pair_flow_0_step,
    &ux_sign_scheduled_transfer_pair_flow_1_step,
    &ux_sign_scheduled_transfer_pair_flow_2_step
);


// UI definitions for signing the transaction, or declining to do so.
UX_STEP_CB(
    ux_sign_scheduled_transfer_flow_0_step,
    pnn,
    // TODO: Sign transaction hash and return it.
    ui_idle(),
    {
      &C_icon_validate_14,
      "Sign TX",
      (char *) displayAccount
    });
UX_STEP_CB(
    ux_sign_scheduled_transfer_flow_1_step,
    pnn,
    sendUserRejection(),
    {
      &C_icon_crossmark,
      "Decline to",
      "sign transaction"
    });
UX_FLOW(ux_sign_scheduled_transfer_flow,
    &ux_sign_scheduled_transfer_flow_0_step,
    &ux_sign_scheduled_transfer_flow_1_step
);


// As long as there are more pairs to process, then stay in the same data/ui flow. If the processed pair was the last
// one, then move on to signing the transaction.
void nextPair(uint8_t *buffer) {

    // The full transaction has been put into the hash, continue to signing flow.
    if (numberOfScheduledAmounts == 0 && scheduledAmountsInCurrentPacket == 0) {

        ux_flow_init(0, ux_sign_scheduled_transfer_flow, NULL);

    } else if (scheduledAmountsInCurrentPacket == 0) {
        // Current packet has been successfully read, but there are still more data to receive.
        sendSuccessNoIdle(0);
    } else {
        // There are still pairs left to be processed in the current packet.
        // Either there are still values left to be processed in the current packet, or we have to send back OK
        // and await a new packet with more values.
        uint64_t timestamp = U8BE(buffer, 0);
        pos += 8;
        bin2dec(displayTimestamp, timestamp);

        uint64_t amount = U8BE(buffer, 8);
        pos += 8;
        bin2dec(displayAmount, amount);

        // We read one more, so count down.
        scheduledAmountsInCurrentPacket -= 1;

        ux_flow_init(0, ux_sign_scheduled_transfer_pair_flow, NULL);
    }
}

// UI definitions for displaying the transaction contents for verification before approval by
// the user.
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
      .text = (char *) displayStr
    });
UX_STEP_VALID(
    ux_scheduled_transfer_initial_flow_2_step,
    nn,
    sendSuccessNoIdle(0),
    {
      "Continue",
      "with transaction"
    });
UX_FLOW(ux_scheduled_transfer_initial_flow,
    &ux_scheduled_transfer_initial_flow_0_step,
    &ux_scheduled_transfer_initial_flow_1_step,
    &ux_scheduled_transfer_initial_flow_2_step
);

// Timestamp is word 64 (8 bytes), seconds since epoch.
void handleSignTransferWithSchedule(uint8_t *dataBuffer, uint8_t p1, volatile unsigned int *flags) {

    // P1 == 0x00 indicates that it is the first packet.
    // TODO Add protection so that we enforce the correct order of packets, so that we don't end up signing some
    // incorrect data.
    // TODO Define p1 values instead of having magic variables.
    if (p1 == 0x00) {
        parseAccountSignatureKeyPath(dataBuffer);
        dataBuffer += 2;

        uint8_t numberOfScheduledAmountsArray[1];
        os_memmove(numberOfScheduledAmountsArray, dataBuffer, 1);
        numberOfScheduledAmounts = numberOfScheduledAmountsArray[0];
        dataBuffer += 1;

        os_memmove(displayAccount, "with #", 6);
        bin2dec(displayAccount + 6, keyPath->accountIndex);

        // Extract the destination address and add to hash.
        uint8_t toAddress[32];
        os_memmove(toAddress, dataBuffer, 32);
        dataBuffer += 32;
        // cx_hash((cx_hash_t *) &hash, 0, toAddress, 32, NULL, 0);

        // Used in display of recipient address
        toHex(toAddress, sizeof(toAddress), displayStr);

        // Display the transaction information to the user (recipient address and amount to be sent).
        ux_flow_init(0, ux_scheduled_transfer_initial_flow, NULL);

        // Tell the main process to wait for a button press.
        *flags |= IO_ASYNCH_REPLY;
    } else {
        // Load the scheduled transfer information.
        // First 8 bytes is the timestamp, the following 8 bytes is the amount.

        // We have room for 255 bytes, so 240 = 15 * 16, i.e. 15 pairs in each packet. Determine how many pairs are
        // in the current packet.
        if (numberOfScheduledAmounts <= 15) {
            scheduledAmountsInCurrentPacket = numberOfScheduledAmounts;
            numberOfScheduledAmounts = 0;
        } else {
            // The maximum is available in the packet.
            scheduledAmountsInCurrentPacket = 15;
            numberOfScheduledAmounts -= 15;
        }

        pos = 0;

        os_memmove(buffer, dataBuffer, scheduledAmountsInCurrentPacket * 16);

        nextPair(buffer);

        // Tell the main process to wait for a button press.
        *flags |= IO_ASYNCH_REPLY;
    }
}