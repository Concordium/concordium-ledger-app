
#ifndef _CONCORDIUM_APP_UPDATE_FINALIZATION_COMITTEE_PARAMETERS_H_
#define _CONCORDIUM_APP_UPDATE_FINALIZATION_COMITTEE_PARAMETERS_H_

/**
 * Handles the signing flow, including updating the display, for the 'update finalization comittee parameters'
 * update instruction.
 * @param cdata please see /doc/ins_update_finalization_comittee_parameters.md for details
 */
void handleSignUpdateFinalizationCommitteeParameters(uint8_t *cdata, volatile unsigned int *flags);

typedef struct {
    uint8_t minFinalizers[11];
    uint8_t maxFinalizers[11];
    uint8_t relativeStakeThreshold[8];
    char updateTypeText[32];
} signUpdateFinalizationCommitteeParametersContext_t;

#endif
