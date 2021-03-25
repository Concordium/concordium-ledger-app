#ifndef _CONCORDIUM_APP_UPDATE_BAKER_STAKE_H_
#define _CONCORDIUM_APP_UPDATE_BAKER_STAKE_H_

/**
 * Handles the signing flow, including updating the display, for the 'update baker stake'
 * account transaction.
 * @param cdata please see /doc/ins_update_baker_stake.md for details
 */
void handleSignUpdateBakerStake(uint8_t *cdata, volatile unsigned int *flags);

#endif
