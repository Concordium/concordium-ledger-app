#ifndef _CONCORDIUM_APP_UPDATE_POOL_PARAMETERS_H_
#define _CONCORDIUM_APP_UPDATE_POOL_PARAMETERS_H_

/**
 * Handles the signing flow, including updating the display, for the 'update pool parameters'
 * update instruction.
 * @param cdata please see /doc/ins_update_pool_parameters.md for details
 */
void handleSignUpdatePoolParameters(uint8_t *cdata, uint8_t p1, volatile unsigned int *flags, bool isInitialCall);

typedef enum {
    TX_UPDATE_POOL_PARAMETERS_INITIAL = 67,
    TX_UPDATE_POOL_PARAMETERS_RANGES = 68,
    TX_UPDATE_POOL_PARAMETERS_EQUITY = 69,
} updatePoolParametersState_t;

typedef struct {
    union {
        uint8_t passiveFinalizationRewardCommissionRate[8];
        uint8_t finalizationRewardCommissionRateMax[8];
    };
    union {
        uint8_t passiveBakingRewardCommissionRate[8];
        uint8_t bakingRewardCommissionRateMax[8];
    };
    union {
        uint8_t passiveTransactionFeeCommissionRate[8];
        uint8_t transactionFeeCommissionRateMax[8];
    };
    uint8_t finalizationRewardCommissionRateMin[8];
    uint8_t bakingRewardCommissionRateMin[8];
    uint8_t transactionFeeCommissionRateMin[8];
    uint8_t minimumEquityCapital[26];
    uint8_t capitalBound[8];
    uint8_t leverageBound[44];
    updatePoolParametersState_t state;
} signUpdatePoolParametersContext_t;

#endif
