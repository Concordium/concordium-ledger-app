#include <os.h>
#include <os_io_seproxyhal.h>
#include "cx.h"
#include <stdint.h>
#include "menu.h"
#include "util.h"
#include <string.h>

static accountSubtreePath_t *keyPath = &path;
static tx_state_t *tx_state = &tx_context;

// TODO: Optimize memory management. Each instruction file should have its own structure, and then we build a
// TODO global variable that supports the different types of transactions in a union. Then the memory footprint will
// TODO be no greater than what a single file uses. Currently each file uses more and more RAM.

static char displayStr[65];
static uint8_t displayAccount[8];
static uint8_t remainingNumberOfScheduledAmounts;
static uint8_t scheduledAmountsInCurrentPacket;

static uint8_t displayAmount[25];
static uint8_t displayTimestamp[25];

// Buffer to hold the incoming databuffer so that we can iterate over it.
static uint8_t buffer[255];
static uint8_t pos;

void processNextScheduledAmount(uint8_t *buffer);
void signTransferWithScheduleHash();

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
        "Amount (uGTU)",
        (char *) displayAmount
    });
UX_STEP_CB(
    ux_sign_scheduled_transfer_pair_flow_2_step,
    nn,
    processNextScheduledAmount(buffer),
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
    signTransferWithScheduleHash(),
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

// Hashes transaction, signs it and sends the signature back to the computer.
void signTransferWithScheduleHash() {
    // Reset initialization status, as we are done processing the current transaction.
    tx_state->initialized = false;

    cx_hash((cx_hash_t *) &tx_state->hash, CX_LAST, NULL, 0, tx_state->transactionHash, 32);

    uint8_t signedHash[64];
    signTransactionHash(keyPath->identity, keyPath->accountIndex, tx_state->transactionHash, signedHash);

    os_memmove(G_io_apdu_buffer, signedHash, sizeof(signedHash));
    sendSuccess(sizeof(signedHash));
}

void processNextScheduledAmount(uint8_t *buffer) {
    // The full transaction has been added to the hash, so we can continue to the signing process.
    if (remainingNumberOfScheduledAmounts == 0 && scheduledAmountsInCurrentPacket == 0) {
        ux_flow_init(0, ux_sign_scheduled_transfer_flow, NULL);
    } else if (scheduledAmountsInCurrentPacket == 0) {
        // Current packet has been successfully read, but there are still more data to receive. Ask the computer
        // for more data.
        sendSuccessNoIdle(0);
    } else {
        // The current packet still has additional timestamp/amount pairs to be added to the hash and
        // displayed for the user.
        uint64_t timestamp = U8BE(buffer, pos);
        cx_hash((cx_hash_t *) &tx_state->hash, 0, buffer + pos, 8, NULL, 0);
        pos += 8;
        bin2dec(displayTimestamp, timestamp);

        uint64_t amount = U8BE(buffer, pos);
        cx_hash((cx_hash_t *) &tx_state->hash, 0, buffer + pos, 8, NULL, 0);
        pos += 8;
        bin2dec(displayAmount, amount);

        // We read one more scheduled amount, so count down to keep track of remaining to process.
        scheduledAmountsInCurrentPacket -= 1;

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
        parseAccountSignatureKeyPath(dataBuffer);
        dataBuffer += 2;

        os_memmove(displayAccount, "with #", 6);
        bin2dec(displayAccount + 6, keyPath->accountIndex);

        uint8_t numberOfScheduledAmountsArray[1];
        os_memmove(numberOfScheduledAmountsArray, dataBuffer, 1);
        remainingNumberOfScheduledAmounts = numberOfScheduledAmountsArray[0];
        dataBuffer += 1;

        // Initialize the transaction hash object.
        cx_sha256_init(&tx_state->hash);

        // Add transaction header to the hash.
        cx_hash((cx_hash_t *) &tx_state->hash, 0, dataBuffer, 60, NULL, 0);
        dataBuffer += 60;

        // Transaction payload/body comes right after the transaction header. First byte determines the transaction kind.
        uint8_t transactionKind[1];
        os_memmove(transactionKind, dataBuffer, 1);
        dataBuffer += 1;

        // Extract the destination address and add to hash.
        uint8_t toAddress[32];
        os_memmove(toAddress, dataBuffer, 32);
        dataBuffer += 32;
        cx_hash((cx_hash_t *) &tx_state->hash, 0, toAddress, 32, NULL, 0);

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
        if (remainingNumberOfScheduledAmounts <= 15) {
            scheduledAmountsInCurrentPacket = remainingNumberOfScheduledAmounts;
            remainingNumberOfScheduledAmounts = 0;
        } else {
            // The maximum is available in the packet.
            scheduledAmountsInCurrentPacket = 15;
            remainingNumberOfScheduledAmounts -= 15;
        }

        // Reset pointer keeping track of where we are in the current packet being processed.
        pos = 0;

        os_memmove(buffer, dataBuffer, scheduledAmountsInCurrentPacket * 16);
        processNextScheduledAmount(buffer);

        // Tell the main process to wait for a button press.
        *flags |= IO_ASYNCH_REPLY;
    }
}