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
    // Used both for l pool rate and max of range:
    uint8_t finalizationRewardCommissionRate[43];
    uint8_t bakingRewardCommissionRate[43];
    uint8_t transactionFeeCommissionRate[43];
    uint8_t finalizationRewardCommissionRateMin[43];
    uint8_t bakingRewardCommissionRateMin[43];
    uint8_t transactionFeeCommissionRateMin[43];
    uint8_t minimumEquityCapital[26];
    uint8_t capitalBound[43];
    uint8_t leverageBound[43];
    updatePoolParametersState_t state;
} signUpdatePoolParametersContext_t;

#endif