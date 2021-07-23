#ifndef _CONCORDIUM_APP_ADD_BAKER_OR_UPDATE_BAKER_KEYS_H_
#define _CONCORDIUM_APP_ADD_BAKER_OR_UPDATE_BAKER_KEYS_H_

/**
 * Method for signing either an 'AddBaker' transaction or an 'UpdateBakerKeys' transaction. The method
 * is shared as the latter transaction is almost a subset of the prior (except a difference in transaction kind byte).
 * 
 * @param cdata please see /doc/ins_add_baker_or_update_baker_keys.md for details
 * @param p2 use 0x00 to run add baker flow, use 0x01 to update baker keys.
 */
void handleSignAddBakerOrUpdateBakerKeys(uint8_t *cdata, uint8_t p1, uint8_t p2, volatile unsigned int *flags, bool isInitialCall);

typedef enum {
    ADD_BAKER_INITIAL = 0,
    ADD_BAKER_VERIFY_KEYS = 17,
    ADD_BAKER_PROOFS_AMOUNT_RESTAKE = 18
} addBakerState_t;

typedef struct {
    uint8_t amount[26];
    uint8_t restake[4];
    addBakerState_t state;
} signAddBakerContext_t;

typedef struct {
    uint8_t amount[26];
} signUpdateBakerStakeContext_t;

#endif
