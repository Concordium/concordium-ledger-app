#ifndef _CONCORDIUM_APP_ACCOUNT_TRANSFER_H_
#define _CONCORDIUM_APP_ACCOUNT_TRANSFER_H_

/**
 * Handles the signing flow, including updating the display, for the 'simple transfer'
 * account transaction.
 * @param cdata please see /doc/ins_transfer.md for details
 */
void handleSignTransfer(uint8_t *cdata, uint8_t p1, uint8_t dataLength, volatile unsigned int *flags, bool isInitialCall);


typedef enum {
    TX_TRANSFER_INITIAL = 51,
    TX_TRANSFER_MEMO_LENGTH = 52,
    TX_TRANSFER_MEMO = 53
} simpleTransferState_t;

typedef struct {
    uint8_t memo[255];
    uint32_t memoLength;
    unsigned char displayStr[52];
    uint8_t displayAmount[26];
    simpleTransferState_t state;
} signTransferContext_t;

#endif
