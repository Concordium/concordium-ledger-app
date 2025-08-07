#pragma once

/**
 * Handles the derivation and export of account and governance public keys.
 * @param cdata please see /doc/ins_public_key.md for details
 * @param p1 use 0x00 to let the user validate the export on the screen, and 0x01
 * to silently export the public-key without user interaction.
 * @param p2 use 0x00 to only export the public-key, and use 0x01 to also
 * export the signature on the public-key signed with the corresponding private-key.
 */
void handleGetPublicKey(uint8_t *cdata,
                        uint8_t p1,
                        uint8_t p2,
                        uint8_t lc,
                        volatile unsigned int *flags);
void sendPublicKey(bool compare);

typedef struct {
    uint8_t display[21];
    char publicKey[68];
    bool signPublicKey;
} exportPublicKeyContext_t;
