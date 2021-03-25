#ifndef _CONCORDIUM_APP_UPDATE_BAKER_STAKE_THRESHOLD_H_
#define _CONCORDIUM_APP_UPDATE_BAKER_STAKE_THRESHOLD_H_

/**
 * Handles the signing flow for an 'UpdateBakerStakeThreshold' transaction. It validates that
 * the correct UpdateType is supplied (9) and will fail otherwise.
 * 
 * @param cdata please see /doc/ins_update_baker_stake_threshold.md for details
 */
void handleSignUpdateBakerStakeThreshold(uint8_t *cdata, volatile unsigned int *flags);

#endif
