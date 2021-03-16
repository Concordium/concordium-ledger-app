#ifndef _CONCORDIUM_APP_UPDATE_BAKER_STAKE_THRESHOLD_H_
#define _CONCORDIUM_APP_UPDATE_BAKER_STAKE_THRESHOLD_H_

/**
 * Handles the signing flow for an 'UpdateBakerStakeThreshold' transaction. It validates that
 * the correct UpdateType is supplied (9) and will fail otherwise.
 * 
 * @param dataBuffer buffer contanining the key derivation path, the update instruction header,
 * the update type and the baker stake threshold
 */
void handleSignUpdateBakerStakeThreshold(uint8_t *dataBuffer, volatile unsigned int *flags);

#endif
