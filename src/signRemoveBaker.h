#ifndef _CONCORDIUM_APP_ACCOUNT_REMOVE_BAKER_H_
#define _CONCORDIUM_APP_ACCOUNT_REMOVE_BAKER_H_

/**
 * Handles the signing flow, including updating the display, for the 'remove baker'
 * account transaction.
 * @param cdata please see /doc/ins_remove_baker.md for details
 */
void handleSignRemoveBaker(uint8_t *cdata, volatile unsigned int *flags);

#endif
