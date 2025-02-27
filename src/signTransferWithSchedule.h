#pragma once

/**
 * Handles the signing flow for the transfer with schedule account transaction.
 * @param cdata please see /doc/ins_transfer_with_schedule.md for details
 * @param p1 0x00 for the initial packet containing key derivation path, account transaction header,
 * transaction kind, recipient address and the number of scheduled transfers to make, 0x01 when
 * sending pairs of scheduled amounts.
 */
void handleSignTransferWithSchedule(uint8_t *cdata,
                                    uint8_t p1,
                                    uint8_t lc,
                                    volatile unsigned int *flags,
                                    bool isInitialCall);

void handleSignTransferWithScheduleAndMemo(uint8_t *cdata,
                                           uint8_t p1,
                                           uint8_t dataLength,
                                           volatile unsigned int *flags,
                                           bool isInitialCall);

typedef enum {
    TX_TRANSFER_WITH_SCHEDULE_INITIAL = 28,
    TX_TRANSFER_WITH_SCHEDULE_TRANSFER_PAIRS = 29,
    TX_TRANSFER_WITH_SCHEDULE_MEMO_START = 55,
    TX_TRANSFER_WITH_SCHEDULE_MEMO = 56,
} transferWithScheduleState_t;

typedef struct {
    uint8_t transactionType;
    transferWithScheduleState_t state;

    unsigned char displayStr[57];
    uint8_t remainingNumberOfScheduledAmounts;
    uint8_t scheduledAmountsInCurrentPacket;

    uint8_t displayAmount[30];
    uint8_t displayTimestamp[25];

    tm time;

    // Buffer to hold the incoming databuffer so that we can iterate over it.
    uint8_t buffer[255];
    uint8_t pos;
} signTransferWithScheduleContext_t;

void processNextScheduledAmount(uint8_t *buffer);