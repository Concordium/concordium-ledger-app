#ifndef _CONCORDIUM_APP_EXPORT_DATA_H_
#define _CONCORDIUM_APP_EXPORT_DATA_H_

/**
  * Handles the export of the prfKey, idCredSec, sigRetrievelRandomness and AttributeRandomness.
  * @param cdata please see /doc/export_private_identity_data.md for details
  */
void handleExportPrivateIdentityData(uint8_t *dataBuffer, uint8_t p1, uint8_t dataLength, volatile unsigned int *flags, bool isInitialCall);

typedef enum {
    EXPORT_DATA_INITIAL = 105,
    EXPORT_DATA_ATTRIBUTES = 106,
} exportDataState_t;

typedef struct {
    uint8_t identityDisplay[15];
    uint8_t attributes[235];
    uint32_t coinType;
    uint32_t identityProvider;
    uint32_t identity;
    uint32_t credCounter;
    uint32_t attributeCount;
    uint8_t outputLength;
    uint8_t output[256];
    uint8_t attributeIndex;
    exportDataState_t state;
} exportDataContext_t;

#endif
