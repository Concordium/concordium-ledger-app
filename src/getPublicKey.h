#include "os.h"
#include "cx.h"
#include "globals.h"

/**
 * Handles the derivation and export of account and governance public keys.
 * @param cdata please see /doc/ins_public_key.md for details
 * @param p1 use 0x00 to let the user validate the export on the screen, and 0x01
 * to silently export the public-key without user interaction.
 * @param p2 use 0x00 to only export the public-key, and use 0x01 to also
 * export the signature on the public-key signed with the corresponding private-key.
 */ 
void handleGetPublicKey(uint8_t *cdata, uint8_t p1, uint8_t p2, volatile unsigned int *flags);
