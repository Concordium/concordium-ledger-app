#ifndef _CONCORDIUM_APP_UPDATE_AUTHORIZATIONS_H_
#define _CONCORDIUM_APP_UPDATE_AUTHORIZATIONS_H_

/**
 * Handles the signing flow for the two authorization update instructions:
 *
 *  - Update level 2 keys with root keys
 *  - Update level 2 keys with level 1 keys
 *
 * @param cdata please see /doc/ins_update_authorizations.md
 * @param p1 please see /doc/ins_update_authorizations.md
 * @param updateType the update type value, should be '12' for updating with root keys, and '14' for updating with level
 * 1 keys
 */
void handleSignUpdateAuthorizations(
    uint8_t *cdata,
    uint8_t p1,
    uint8_t p2,
    uint8_t updateType,
    uint8_t dataLength,
    volatile unsigned int *flags,
    bool isInitialCall);

typedef enum {
    TX_UPDATE_AUTHORIZATIONS_INITIAL = 30,
    TX_UPDATE_AUTHORIZATIONS_PUBLIC_KEY = 31,
    TX_UPDATE_AUTHORIZATIONS_ACCESS_STRUCTURE_SIZE = 32,
    TX_UPDATE_AUTHORIZATIONS_ACCESS_STRUCTURE_INDEX = 33,
    TX_UPDATE_AUTHORIZATIONS_ACCESS_STRUCTURE_THRESHOLD = 34
} updateAuthorizationsState_t;

// To add support for additional access structures with the update authorizations
// transaction, you simply have to add a new item to the enum prior to the 'END' entry,
// and update the enum -> string method in signUpdateAuthorizations.c.
typedef enum {
    AUTHORIZATION_EMERGENCY,
    AUTHORIZATION_PROTOCOL,
    AUTHORIZATION_ELECTION_DIFFICULTY,
    AUTHORIZATION_EURO_PER_ENERGY,
    AUTHORIZATION_MICRO_GTU_PER_EURO,
    AUTHORIZATION_FOUNDATION_ACCOUNT,
    AUTHORIZATION_MINT_DISTRIBUTION,
    AUTHORIZATION_TRANSACTION_FEE_DISTRIBUTION,
    AUTHORIZATION_GAS_REWARDS,
    AUTHORIZATION_BAKER_STAKE_THRESHOLD,
    AUTHORIZATION_ADD_ANONYMITY_REVOKER,
    AUTHORIZATION_ADD_IDENTITY_PROVIDER,  // Last for v0
    AUTHORIZATION_COOLDOWN_PARAMETERS,
    AUTHORIZATION_TIME_PARAMETERS  // Last for v1
} authorizationType_e;

typedef struct {
    uint16_t publicKeyListLength;
    uint16_t publicKeyCount;
    char publicKey[68];
    uint16_t accessStructureSize;
    uint8_t title[29];
    uint8_t displayKeyIndex[6];
    uint8_t type[24];

    uint8_t processedCount;

    uint8_t buffer[255];
    int bufferPointer;

    updateAuthorizationsState_t state;
    authorizationType_e authorizationType;
    authorizationType_e lastAuthorizationType;
} signUpdateAuthorizations_t;

#endif
