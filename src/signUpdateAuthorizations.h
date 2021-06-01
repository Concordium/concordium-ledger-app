/**
 * Handles the signing flow for the two authorization update instructions:
 * 
 *  - Update level 2 keys with root keys
 *  - Update level 2 keys with level 1 keys
 * 
 * @param cdata please see /doc/ins_update_authorizations.md
 * @param p1 please see /doc/ins_update_authorizations.md
 * @param updateType the update type value, should be '12' for updating with root keys, and '14' for updating with level 1 keys
 */
void handleSignUpdateAuthorizations(uint8_t *cdata, uint8_t p1, uint8_t updateType, uint8_t dataLength, volatile unsigned int *flags);