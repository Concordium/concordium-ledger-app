#ifndef _CONCORDIUM_APP_MESSAGE_H_
#define _CONCORDIUM_APP_MESSAGE_H_

/**
 * Handles the signing flow, including updating the display, for signing a message.
 * @param cdata please see /doc/ins_message.md for details
 */
void handleSignMessage(
    uint8_t *cdata,
    uint8_t p1,
    uint8_t p2,
    uint8_t dataLength,
    volatile unsigned int *flags,
    bool isInitialCall);

typedef enum {
    SIGN_MESSAGE_INITIAL = 110,
    SIGN_MESSAGE_CONTINUED = 111,
} signMessageState_t;

typedef struct {
    uint8_t initialP2;
    bool displayStart;
    uint32_t messageLength;
    signMessageState_t state;
    char displayHeader[14];
    char display[542];
    unsigned char displaySigner[57];
} signMessageContext_t;

#endif
