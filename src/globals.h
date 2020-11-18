#include "os.h"
#include "ux.h"
#include "cx.h"
#include <stdbool.h>

#ifndef _GLOBALS_H_
#define _GLOBALS_H_

// TODO: The concordium coin type value has to be set to the value we get in SLIP44.
#define CONCORDIUM_COIN_TYPE 691
#define CONCORDIUM_PURPOSE 583

typedef struct {
    uint8_t identity;
    uint8_t accountIndex;
} accountSubtreePath_t;
extern accountSubtreePath_t path;

// Helper object used when computing the hash of a transaction.
typedef struct {
    cx_sha256_t hash;
    uint8_t transactionHash[32];
    bool initialized;
} tx_state_t;

// Each instruction's state has to have its own struct here that is put in the global union below. This translates
// into each handler file having its own struct here.
typedef struct {
    unsigned char displayStr[52];

    uint8_t displayAccount[10];
    uint8_t displayAmount[21];

    // The signature is 64 bytes, in hexadecimal that is 128 bytes + 1 for string terminator.
    char signatureAsHex[129];
    tx_state_t tx_state;
} signTransferContext_t;

typedef struct {
    unsigned char displayStr[52];
    uint8_t displayAccount[10];
    uint8_t remainingNumberOfScheduledAmounts;
    uint8_t scheduledAmountsInCurrentPacket;

    uint8_t displayAmount[21];
    uint8_t displayTimestamp[25];

    // Buffer to hold the incoming databuffer so that we can iterate over it.
    uint8_t buffer[255];
    uint8_t pos;

    tx_state_t tx_state;
} signTransferWithScheduleContext_t;

// TODO This could be heavily optimized, but let's skip that for now unless we run into trouble.
typedef struct {
    uint8_t displayAccount[10];
    uint8_t numberOfVerificationKeys;

    char accountVerificationKey[65];

    uint8_t signatureThreshold[4];
    char regIdCred[97];

    uint8_t identityProviderIdentity[4];
    uint8_t anonymityRevocationThreshold[4];

    uint16_t anonymityRevocationListLength;

    uint8_t arIdentity[11];
    uint8_t encIdCredPubShare[1];

    uint8_t validTo[8];
    uint8_t createdAt[8];

    uint16_t attributeListLength;

    char attributeTag[19];

    uint8_t attributeValueLength;
    uint8_t attributeValue[256];

} signCredentialDeploymentContext_t;

// As the Ledger device is very limited on memory, the context of each instruction is stored in a
// shared global union, so that we use no more memory than that of the most space using instruction context.
typedef union {
    signTransferWithScheduleContext_t signTransferWithScheduleContext;
    signTransferContext_t signTransferContext;
    signCredentialDeploymentContext_t signCredentialDeploymentContext;
} instructionContext;
extern instructionContext global;

#endif