#ifndef _CONCORDIUM_APP_UPDATE_TIMEOUT_PARAMETERS_H_
#define _CONCORDIUM_APP_UPDATE_TIMEOUT_PARAMETERS_H_

/**
 * Handles the signing flow, including updating the display, for the 'update timeout parameters'
 * update instruction.
 * @param cdata please see /doc/ins_update_timeout_parameters.md for details
 */
void handleSignTimeoutParameters(uint8_t *cdata, volatile unsigned int *flags);

typedef struct {
    uint8_t timeoutBase[21];
    uint8_t increaseTimeoutRatio[44];
    uint8_t decreaseTimeoutRatio[44];
    char updateTypeText[32];
} signUpdateTimeoutParametersContext_t;

#endif
