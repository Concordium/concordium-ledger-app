#ifndef _CONCORDIUM_APP_HIGHER_LEVEL_KEY_UPDATE_H_
#define _CONCORDIUM_APP_HIGHER_LEVEL_KEY_UPDATE_H_

/**
 * Handles the signing flow for three types of update instructions:
 * 
 *  - Update root keys with root keys
 *  - Update level 1 keys with root keys
 *  - Update level 1 keys with level 1 keys
 * 
 * They are all handled together with a shared function, as their serializations are almost
 * equal.
 * @param cdata please see /doc/ins_higher_level_keys.md
 * @param p1 please see /doc/ins_higher_level_keys.md
 * @param updateType the update type value, should be '10' for updating root keys with root keys, '11' for updating level 1
 * keys with root keys, and '13' for updating level 1 keys with level 1 keys.
 */
void handleSignHigherLevelKeys(uint8_t *cdata, uint8_t p1, uint8_t updateType, volatile unsigned int *flags, bool isInitialCall);

/**
 * The 'RootUpdate' update instruction payload is prefixed by
 * a byte depending on the type of key update, which is represented
 * by this enum.
 */
typedef enum {
    ROOT_UPDATE_ROOT,
    ROOT_UPDATE_LEVEL_1,
    ROOT_UPDATE_LEVEL_2
} rootUpdatePrefix_e;

typedef enum {
    LEVEL1_UPDATE_LEVEL_1,
    LEVEL1_UPDATE_LEVEL_2
} level1UpdatePrefix_e;

typedef enum {
    TX_UPDATE_KEYS_INITIAL = 19,
    TX_UPDATE_KEYS_KEY = 20,
    TX_UPDATE_KEYS_THRESHOLD = 21
} updateKeysState_t;

typedef struct {
    uint8_t type[25];
    uint16_t numberOfUpdateKeys;
    char updateVerificationKey[68];
    uint8_t threshold[6];
    updateKeysState_t state;
} signUpdateKeysWithRootKeysContext_t;

#endif
