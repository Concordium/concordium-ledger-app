
#ifndef _CONCORDIUM_APP_UPDATE_BLOCK_ENERGY_LIMIT_H_
#define _CONCORDIUM_APP_UPDATE_BLOCK_ENERGY_LIMIT_H_

/**
 * Handles the signing flow, including updating the display, for the 'update block energy limit'
 * update instruction.
 * @param cdata please see /doc/ins_update_block_energy_limit.md for details
 */
void handleSignUpdateBlockEnergyLimit(uint8_t *cdata, volatile unsigned int *flags);

typedef struct {
    uint8_t blockEnergyLimit[22];
} signUpdateBlockEnergyLimitContext_t;

#endif
