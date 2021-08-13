#ifndef _CONCORDIUM_APP_ADD_ANONYMITY_REVOKER_H_
#define _CONCORDIUM_APP_ADD_ANONYMITY_REVOKER_H_

#include "descriptionView.h"

/**
 * Handles the signing flow, including updating the display, for the 'add anonymity revoker'
 * update instruction.
 * @param cdata please see /doc/ins_add_anonymity_revoker.md for details
 */
void handleSignAddAnonymityRevoker(uint8_t *cdata, uint8_t p1, uint8_t dataLength, volatile unsigned int *flags, bool isInitialCall);

typedef enum {
    TX_ADD_ANONYMITY_REVOKER_INITIAL = 40,
    TX_ADD_ANONYMITY_REVOKER_DESCRIPTION_LENGTH = 41,
    TX_ADD_ANONYMITY_REVOKER_DESCRIPTION = 42,
    TX_ADD_ANONYMITY_REVOKER_PUBLIC_KEY = 43,
} addAnonymityRevokerState_t;

typedef struct {
    uint32_t payloadLength;
    char publicKey[97];
    uint8_t arIdentity[5];
    addAnonymityRevokerState_t state;
} signAddAnonymityRevokerContext_t;

#endif
