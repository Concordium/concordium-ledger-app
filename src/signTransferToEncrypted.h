#pragma once

/**
 * Handles the signing flow, including updating the display, for the 'transfer to encrypted'
 * account transaction.
 * @param cdata please see /doc/ins_transfer_to_encrypted.md
 */
void handleSignTransferToEncrypted(uint8_t *cdata, volatile unsigned int *flags);

typedef struct {
    uint8_t amount[30];
} signTransferToEncrypted_t;
