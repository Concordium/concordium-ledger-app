#pragma once

/**
 * Handles the signing flow for a 'Configure Delegation' transaction. It validates
 * that the correct UpdateType is supplied and will fail otherwise.
 * @param cdata please see /doc/ins_configure_delegation.md for details
 */
void handleSignConfigureDelegation(uint8_t *cdata,
                                   uint8_t dataLength,
                                   volatile unsigned int *flags);

typedef struct {
    bool stopDelegation;
    uint8_t displayCapital[30];
    uint8_t displayRestake[4];
    uint8_t displayDelegationTarget[30];
    bool hasCapital;
    bool hasRestakeEarnings;
    bool hasDelegationTarget;
} signConfigureDelegationContext_t;
