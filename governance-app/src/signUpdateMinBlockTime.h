
#ifndef _CONCORDIUM_APP_UPDATE_MIN_BLOCK_TIME_H_
#define _CONCORDIUM_APP_UPDATE_MIN_BLOCK_TIME_H_

/**
 * Handles the signing flow, including updating the display, for the 'update min block time'
 * update instruction.
 * @param cdata please see /doc/ins_update_min_block_time.md for details
 */
void handleSignMinBlockTime(uint8_t *cdata, volatile unsigned int *flags);

typedef struct {
    uint8_t minBlockTime[25];
    char updateTypeText[32];
} signUpdateMinBlockTimeContext_t;

#endif
