#ifndef _CONCORDIUM_APP_UPDATE_COOLDOWN_PARAMETERS_H_
#define _CONCORDIUM_APP_UPDATE_COOLDOWN_PARAMETERS_H_

/**
 * Handles the signing flow, including updating the display, for the 'update cooldown parameters'
 * update instruction.
 * @param cdata please see /doc/ins_update_cooldown_parameters.md for details
 */
void handleSignUpdateCooldownParameters(uint8_t *cdata, volatile unsigned int *flags);

typedef struct {
    uint8_t poolOwnerCooldown[21];
    uint8_t delegatorCooldown[21];
    char updateTypeText[32];
} signUpdateCooldownParametersContext_t;

#endif
