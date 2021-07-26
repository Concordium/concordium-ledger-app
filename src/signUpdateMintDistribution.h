#ifndef _CONCORDIUM_APP_UPDATE_MINT_DISTRIBUTION_H_
#define _CONCORDIUM_APP_UPDATE_MINT_DISTRIBUTION_H_

/**
 * Handles the signing flow, including updating the display, for the 'update mint distribution'
 * update instruction.
 * @param cdata please see /doc/ins_update_mint_distribution.md for details
 */
void handleSignUpdateMintDistribution(uint8_t *cdata, volatile unsigned int *flags);

typedef struct {
    uint8_t mintRate[35];
    uint8_t bakerReward[17];
    uint8_t finalizationReward[17];
} signUpdateMintDistribution_t;

#endif
