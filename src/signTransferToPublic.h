#ifndef _CONCORDIUM_APP_ACCOUNT_TRANSFER_TO_PUBLIC_H_
#define _CONCORDIUM_APP_ACCOUNT_TRANSFER_TO_PUBLIC_H_

/**
 * Handles the signing flow, including updating the display, for the 'transfer to public'
 * account transaction.
 * @param cdata please see /doc/ins_transfer_to_public.md
 */
void handleSignTransferToPublic(uint8_t *cdata, uint8_t p1, uint8_t dataLength, volatile unsigned int *flags);

#endif
