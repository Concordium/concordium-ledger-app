#ifndef _CONCORDIUM_APP_UPDATE_PROTOCOL_H_
#define _CONCORDIUM_APP_UPDATE_PROTOCOL_H_

/**
 * Handles the signing flow, including updating the display, for the 'update protocol'
 * update instruction.
 * @param cdata please see /doc/ins_update_protocol.md for details
 */
void handleSignUpdateProtocol(
    uint8_t *cdata,
    uint8_t p1,
    uint8_t dataLength,
    volatile unsigned int *flags,
    bool isInitialCall);

typedef enum {
    TX_UPDATE_PROTOCOL_INITIAL = 35,
    TX_UPDATE_PROTOCOL_TEXT_LENGTH = 36,
    TX_UPDATE_PROTOCOL_TEXT = 37,
    TX_UPDATE_PROTOCOL_SPECIFICATION_HASH = 38,
    TX_UPDATE_PROTOCOL_AUXILIARY_DATA = 39
} updateProtocolState_t;

typedef enum { MESSAGE, SPECIFICATION_URL, TEXT_STATE_END } textState_t;

typedef struct {
    uint64_t payloadLength;
    uint64_t textLength;
    uint8_t buffer[255];
    textState_t textState;
    updateProtocolState_t state;
    char specificationHash[68];
} signUpdateProtocolContext_t;

#endif
