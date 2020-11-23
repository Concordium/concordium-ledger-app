#include "os.h"
#include "ux.h"
#include "cx.h"
#include <stdbool.h>

#ifndef _GLOBALS_H_
#define _GLOBALS_H_

// TODO: The concordium coin type value has to be set to the value we get in SLIP44.
#define CONCORDIUM_COIN_TYPE 691
#define CONCORDIUM_PURPOSE 583

#define SW_INVALID_STATE 0x6B01
#define SW_INVALID_PATH  0x6B02

#define MAX_CDATA_LENGTH 255

typedef enum {
    TX_INITIAL,
    TX_VERIFICATION_KEY,
    TX_SIGNATURE_THRESHOLD,
    TX_AR_IDENTITY,
    TX_CREDENTIAL_DATES,
    TX_ATTRIBUTE_TAG,
    TX_ATTRIBUTE_VALUE,
    TX_LENGTH_OF_PROOFS,
    TX_PROOFS
} protocolState_t;

typedef struct {
    uint8_t identity;
    uint8_t accountIndex;

    // Max length of path is 8. Currently we expect to receive the root, i.e. purpose and cointype as well.
    // This could be refactored into having those values hardcoded if we determine they will be static.
    uint8_t pathLength;
    uint32_t keyDerivationPath[8];
} keyDerivationPath_t;
extern keyDerivationPath_t path;

// Helper object used when computing the hash of a transaction.
typedef struct {
    cx_sha256_t hash;
    uint8_t transactionHash[32];
    bool initialized;
} tx_state_t;
extern tx_state_t global_tx_state;

// Each instruction's state has to have its own struct here that is put in the global union below. This translates
// into each handler file having its own struct here.
typedef struct {
    unsigned char displayStr[52];
    uint8_t displayAmount[21];

    // The signature is 64 bytes, in hexadecimal that is 128 bytes + 1 for string terminator.
    char signatureAsHex[129];
    tx_state_t tx_state;
} signTransferContext_t;

typedef struct {
    unsigned char displayStr[52];
    uint8_t remainingNumberOfScheduledAmounts;
    uint8_t scheduledAmountsInCurrentPacket;

    uint8_t displayAmount[21];
    uint8_t displayTimestamp[25];

    // Buffer to hold the incoming databuffer so that we can iterate over it.
    uint8_t buffer[255];
    uint8_t pos;

    tx_state_t tx_state;
} signTransferWithScheduleContext_t;

typedef struct {
    uint8_t type;
    uint8_t numberOfVerificationKeys;

    char accountVerificationKey[65];

    uint8_t signatureThreshold[4];
    char regIdCred[97];

    uint8_t identityProviderIdentity[4];
    uint8_t anonymityRevocationThreshold[4];

    uint16_t anonymityRevocationListLength;

    uint8_t arIdentity[11];
    char encIdCredPubShare[192];

    uint8_t validTo[8];
    uint8_t createdAt[8];

    uint16_t attributeListLength;

    cx_sha256_t attributeHash;
    uint8_t attributeValueLength;
    char attributeHashDisplay[65];

    uint32_t proofLength;
    uint8_t buffer[255];

    protocolState_t state;
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