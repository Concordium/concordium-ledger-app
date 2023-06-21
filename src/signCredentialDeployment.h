#ifndef _CONCORDIUM_APP_SIGN_CREDENTIAL_DEPLOYMENT_H_
#define _CONCORDIUM_APP_SIGN_CREDENTIAL_DEPLOYMENT_H_

#include "cx.h"
#include "os.h"

void handleSignCredentialDeployment(
    uint8_t *dataBuffer,
    uint8_t p1,
    uint8_t p2,
    volatile unsigned int *flags,
    bool isInitialCall);

void handleSignUpdateCredential(
    uint8_t *dataBuffer,
    uint8_t p1,
    uint8_t p2,
    volatile unsigned int *flags,
    bool isInitialCall);

typedef enum {
    TX_CREDENTIAL_DEPLOYMENT_INITIAL = 4,
    TX_CREDENTIAL_DEPLOYMENT_VERIFICATION_KEYS_LENGTH = 5,
    TX_CREDENTIAL_DEPLOYMENT_VERIFICATION_KEY = 6,
    TX_CREDENTIAL_DEPLOYMENT_SIGNATURE_THRESHOLD = 7,
    TX_CREDENTIAL_DEPLOYMENT_AR_IDENTITY = 8,
    TX_CREDENTIAL_DEPLOYMENT_CREDENTIAL_DATES = 9,
    TX_CREDENTIAL_DEPLOYMENT_ATTRIBUTE_TAG = 10,
    TX_CREDENTIAL_DEPLOYMENT_ATTRIBUTE_VALUE = 11,
    TX_CREDENTIAL_DEPLOYMENT_LENGTH_OF_PROOFS = 12,
    TX_CREDENTIAL_DEPLOYMENT_PROOFS = 13,
    TX_CREDENTIAL_DEPLOYMENT_NEW_OR_EXISTING = 14
} protocolState_t;

typedef enum {
    TX_UPDATE_CREDENTIAL_INITIAL = 0,
    TX_UPDATE_CREDENTIAL_CREDENTIAL_INDEX = 21,
    TX_UPDATE_CREDENTIAL_CREDENTIAL = 22,
    TX_UPDATE_CREDENTIAL_ID_COUNT = 23,
    TX_UPDATE_CREDENTIAL_ID = 24,
    TX_UPDATE_CREDENTIAL_THRESHOLD = 25
} updateCredentialState_t;

typedef struct {
    uint8_t type;
    uint8_t numberOfVerificationKeys;

    uint8_t credentialDeploymentCount;
    uint8_t credentialIdCount;
    char credentialId[102];
    uint8_t threshold[4];
    updateCredentialState_t updateCredentialState;

    char accountVerificationKey[68];
    uint8_t signatureThreshold[4];

    uint8_t anonymityRevocationThreshold[13];
    uint16_t anonymityRevocationListLength;

    uint8_t validTo[8];
    uint8_t createdAt[8];

    uint16_t attributeListLength;

    cx_sha256_t attributeHash;
    uint8_t attributeValueLength;

    uint32_t proofLength;
    uint8_t accountAddress[57];

    protocolState_t state;
    bool showIntro;
} signCredentialDeploymentContext_t;

#endif
