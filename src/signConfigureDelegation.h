#ifndef _CONCORDIUM_APP_CONFIGURE_DELEGATION_H_
#define _CONCORDIUM_APP_CONFIGURE_DELEGATION_H_

/**
 * Handles the signing flow for an 'Configure Delegation' transaction. It validates
 * that the correct UpdateType is supplied and will fail otherwise.
 * @param cdata please see /doc/ins_configure_delegation.md for details
 */
void handleSignConfigureDelegation(uint8_t *cdata, volatile unsigned int *flags);

typedef struct {
    uint8_t displayCapital[26];
    uint8_t displayRestake[4];
    uint8_t displayDelegationTarget[30];
    bool hasCapital;
    bool hasRestakeEarnings;
    bool hasDelegationTarget;
} signConfigureDelegationContext_t;

#endif
