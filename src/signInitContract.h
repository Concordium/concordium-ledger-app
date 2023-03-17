#ifndef _CONCORDIUM_APP_ACCOUNT_INIT_CONTRACT_H_
#define _CONCORDIUM_APP_ACCOUNT_INIT_CONTRACT_H_

/**
 * Handles the signing flow, including updating the display, for the 'init contract'
 * account transaction.
 * @param cdata please see /doc/ins_init_contract.md for details
 */
void handleSignInitContract(uint8_t *cdata, uint8_t p1, uint8_t dataLength, volatile unsigned int *flags, bool isInitialCall);

typedef enum {
    TX_INIT_CONTRACT_INITIAL = 103,
    TX_INIT_CONTRACT_CONTRACT_NAME = 104,
    TX_INIT_CONTRACT_PARAMETER = 105,
} initContractState_t;

typedef struct {
    // For contractName and parameters
    uint8_t display[255];
    uint8_t displayAmount[26];
    char displayModuleRef[70];
    uint16_t nameLength;
    uint8_t rawParameterLength[2];
    uint8_t paramLength;
    bool displayStart;
    initContractState_t state;
} initContractContext_t;

#endif
