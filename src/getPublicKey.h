#ifndef _CONCORDIUM_APP_GET_PUBLIC_KEY_H_
#define _CONCORDIUM_APP_GET_PUBLIC_KEY_H_

/**
 * Handles the derivation and export of account and governance public keys.
 * @param cdata please see /doc/ins_public_key.md for details
 * @param p1 use 0x00 to let the user validate the export on the screen, and 0x01
 * to silently export the public-key without user interaction.
 * @param p2 use 0x00 to only export the public-key, and use 0x01 to also
 * export the signature on the public-key signed with the corresponding private-key.
 */
void handleGetPublicKey(uint8_t *cdata, uint8_t p1, uint8_t p2, volatile unsigned int *flags);

typedef struct {
    uint8_t display[14];
    char publicKey[68];
    bool signPublicKey;
} exportPublicKeyContext_t;

#endif
