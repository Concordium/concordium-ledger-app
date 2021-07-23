#ifndef _CONCORDIUM_APP_ACCOUNT_ENCRYPTED_AMOUNT_TRANSFER_H_
#define _CONCORDIUM_APP_ACCOUNT_ENCRYPTED_AMOUNT_TRANSFER_H_

/**
 * Handles the signing flow, including updating the display, for the 'encrypted amount transfer'
 * account transaction.
 * @param cdata please see /doc/ins_encrypted_amount_transfer.md
 */
void handleSignEncryptedAmountTransfer(uint8_t *cdata, uint8_t p1, uint8_t dataLength, volatile unsigned int *flags, bool isInitialCall);

typedef enum {
    TX_ENCRYPTED_AMOUNT_TRANSFER_INITIAL = 0,
    TX_ENCRYPTED_AMOUNT_TRANSFER_REMAINING_AMOUNT = 6, 
    TX_ENCRYPTED_AMOUNT_TRANSFER_TRANSFER_AMOUNT = 7,
    TX_ENCRYPTED_AMOUNT_TRANSFER_PROOFS = 8
} encryptedAmountTransferState_t;

typedef struct { 
    uint8_t to[52];
    uint16_t proofSize;
    encryptedAmountTransferState_t state;
} signEncryptedAmountToTransfer_t;

#endif
