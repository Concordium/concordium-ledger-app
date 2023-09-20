#ifndef _CONCORDIUM_APP_UPDATE_GAS_REWARDS_H_
#define _CONCORDIUM_APP_UPDATE_GAS_REWARDS_H_

/**
 * Handles the signing flow, including updating the display, for the 'update GAS rewards'
 * update instruction.
 * @param cdata please see /doc/ins_update_gas_rewards.md for details
 */
void handleSignUpdateGasRewards(uint8_t *cdata, volatile unsigned int *flags);

typedef struct {
    uint8_t gasBaker[8];
    uint8_t gasAccountCreation[8];
    uint8_t gasChainUpdate[8];
} signUpdateGasRewardsContext_t;

#endif
