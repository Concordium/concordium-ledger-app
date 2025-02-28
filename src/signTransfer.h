#pragma once

/**
 * Handles the signing flow, including updating the display, for the 'simple transfer'
 * account transaction.
 * @param cdata please see /doc/ins_transfer.md for details
 */
void handleSignTransfer(uint8_t *cdata, uint8_t lc, volatile unsigned int *flags);

/**
 * Handles the signing flow, including updating the display, for the 'simple transfer with memo'
 * account transaction.
 * @param cdata please see /doc/ins_transfer.md for details
 */
void handleSignTransferWithMemo(uint8_t *cdata,
                                uint8_t p1,
                                uint8_t dataLength,
                                volatile unsigned int *flags,
                                bool isInitialCall);

typedef enum {
    TX_TRANSFER_INITIAL = 49,
    TX_TRANSFER_MEMO_INITIAL = 50,
    TX_TRANSFER_MEMO = 51,
    TX_TRANSFER_AMOUNT = 52
} simpleTransferState_t;

typedef struct {
    unsigned char displayStr[57];
    uint8_t displayAmount[30];
    simpleTransferState_t state;
} signTransferContext_t;
