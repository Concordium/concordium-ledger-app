#ifndef _CONCORDIUM_APP_ACCOUNT_REGISTER_DATA_H_
#define _CONCORDIUM_APP_ACCOUNT_REGISTER_DATA_H_

/**
 * Handles the signing flow, including updating the display, for the 'register data'
 * account transaction.
 * @param cdata please see /doc/ins_register_data.md
 */
void handleSignRegisterData(uint8_t *cdata,
                            uint8_t p1,
                            uint8_t dataLength,
                            volatile unsigned int *flags,
                            bool isInitialCall);

typedef enum {
    TX_REGISTER_DATA_INITIAL = 57,
    TX_REGISTER_DATA_PAYLOAD_START = 58,
    TX_REGISTER_DATA_PAYLOAD = 59,
} registerDataState_t;

typedef struct {
    uint8_t display[255];
    uint16_t dataLength;
    registerDataState_t state;
} signRegisterData_t;

#endif
