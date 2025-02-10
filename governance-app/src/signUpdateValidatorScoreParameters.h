#ifndef _CONCORDIUM_APP_UPDATE_VALIDATOR_SCORE_PARAMETERS_H_
#define _CONCORDIUM_APP_UPDATE_VALIDATOR_SCORE_PARAMETERS_H_

/**
 * Handles the signing flow, including updating the display, for the 'update validater score parameters'
 * update instruction.
 * @param cdata please see /doc/ins_update_validator_score_parameters.md for details
 */
void handleSignUpdateValidatorScoreParameters(uint8_t *cdata, volatile unsigned int *flags);

typedef struct {
    uint8_t max_missed_rounds[21];
} signUpdateValidatorScoreParametersContext_t;

#endif
