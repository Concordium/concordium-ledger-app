#ifndef _CONCORDIUM_APP_UPDATE_TIME_PARAMETERS_H_
#define _CONCORDIUM_APP_UPDATE_TIME_PARAMETERS_H_

/**
 * Handles the signing flow, including updating the display, for the 'update mint distribution'
 * update instruction.
 * @param cdata please see /doc/ins_update_mint_distribution.md for details
 */
void handleSignUpdateTimeParameters(uint8_t *cdata, volatile unsigned int *flags);

typedef struct {
    uint8_t mintRate[35];
    uint8_t rewardPeriodLength[17];
} signUpdateTimeParametersContext_t;

#endif
