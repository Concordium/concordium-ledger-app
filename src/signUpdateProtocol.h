#ifndef _CONCORDIUM_APP_UPDATE_PROTOCOL_H_
#define _CONCORDIUM_APP_UPDATE_PROTOCOL_H_

/**
 * Handles the signing flow, including updating the display, for the 'update protocol'
 * update instruction.
 * @param cdata please see /doc/ins_update_protocol.md for details
 */
void handleSignUpdateProtocol(uint8_t *dataBuffer, uint8_t p1, uint8_t dataLength, volatile unsigned int *flags);

#endif
