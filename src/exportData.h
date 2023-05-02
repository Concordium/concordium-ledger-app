#ifndef _CONCORDIUM_APP_EXPORT_DATA_H_
#define _CONCORDIUM_APP_EXPORT_DATA_H_

/**
  * Handles the export of the prfKey, idCredSec, sigRetrievelRandomness and AttributeRandomness.
  * @param cdata please see /doc/export_private_identity_data.md for details
  */
void handleExportPrivateIdentityData(uint8_t *dataBuffer, uint8_t p1, uint8_t p2, volatile unsigned int *flags, bool isInitialCall);

typedef enum {
    EXPORT_DATA_INITIAL = 105,
    EXPORT_DATA_ATTRIBUTES = 106,
} exportDataState_t;

typedef struct {
    uint8_t displayHeader[22];
    uint8_t display[15];
    bool acceptedAll;
    uint8_t p1;
    uint32_t path[7];
    uint8_t pathLength;
    uint32_t attributeCount;
    exportDataState_t state;
} exportDataContext_t;

#endif
