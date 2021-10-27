#ifndef _CONCORDIUM_APP_VERIFY_ADDRESS_H_
#define _CONCORDIUM_APP_VERIFY_ADDRESS_H_

void handleVerifyAddress(uint8_t *cdata, volatile unsigned int *flags);

typedef struct {
    uint8_t display[14];
    unsigned char address[57];
} verifyAddressContext_t;

#endif
