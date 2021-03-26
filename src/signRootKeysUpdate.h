/**
 * Handles the signing flow for three types of update instructions:
 * 
 *  - Update root keys with root keys
 *  - Update level 1 keys with root keys
 *  - Update level 1 keys with level 1 keys
 * 
 * They are all handled together with a shared function, as their serializations are almost
 * equal.
 */
void handleSignHigherLevelKeys(uint8_t *cdata, uint8_t p1, uint8_t updateType, uint8_t expectedKeyUpdateType, volatile unsigned int *flags);