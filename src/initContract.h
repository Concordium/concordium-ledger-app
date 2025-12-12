#pragma once
typedef enum {
    INIT_CONTRACT_INITIAL = 60,
    INIT_CONTRACT_NAME_FIRST = 61,
    INIT_CONTRACT_NAME_NEXT = 62,
    INIT_CONTRACT_PARAMS_FIRST = 63,
    INIT_CONTRACT_PARAMS_NEXT = 64,
    INIT_CONTRACT_END = 65
} initContractState_t;

/**
 * Handles the INIT_CONTRACT instruction, which initializes a contract
 *
 *
 */
void handleInitContract(uint8_t *cdata, uint8_t p1, uint8_t lc);

typedef struct {
    uint64_t amount;
    uint8_t moduleRef[32];
    char amountDisplay[30];
    char moduleRefDisplay[65];
    uint32_t nameLength;
    uint32_t remainingNameLength;
    uint32_t paramsLength;
    uint32_t remainingParamsLength;
    initContractState_t state;
} initContract_t;
