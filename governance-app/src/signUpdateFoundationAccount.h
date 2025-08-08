#ifndef _CONCORDIUM_APP_UPDATE_FOUNDATION_ACCOUNT_H_
#define _CONCORDIUM_APP_UPDATE_FOUNDATION_ACCOUNT_H_

/**
 * Handles the signing flow, including updating the display, for the 'update foundation account'
 * update instruction.
 * @param cdata please see /doc/ins_update_foundation_account.md for details
 */
void handleSignUpdateFoundationAccount(uint8_t *cdata, volatile unsigned int *flags);

typedef struct {
    uint8_t foundationAccountAddress[57];
    char updateTypeText[32];
} signUpdateFoundationAccountContext_t;

#endif
