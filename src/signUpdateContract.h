#ifndef _CONCORDIUM_APP_ACCOUNT_UPDATE_CONTRACT_H_
#define _CONCORDIUM_APP_ACCOUNT_UPDATE_CONTRACT_H_

/**
 * Handles the signing flow, including updating the display, for the 'update contract'
 * account transaction.
 * @param cdata please see /doc/ins_update_contract.md for details
 */
void handleSignUpdateContract(
    uint8_t *cdata,
    uint8_t p1,
    uint8_t dataLength,
    volatile unsigned int *flags,
    bool isInitialCall);

typedef enum {
    TX_UPDATE_CONTRACT_INITIAL = 100,
    TX_UPDATE_CONTRACT_RECEIVE_NAME = 101,
    TX_UPDATE_CONTRACT_PARAMETER = 102,
} updateContractState_t;

typedef struct {
    // For receiveName and parameters
    uint8_t display[255];
    uint8_t displayAmount[26];
    uint8_t displayIndex[26];
    uint8_t displaySubindex[26];
    uint16_t nameLength;
    uint8_t rawParameterLength[2];
    uint8_t paramLength;
    bool displayStart;
    updateContractState_t state;
} updateContractContext_t;

#endif
