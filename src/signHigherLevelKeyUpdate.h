/**
 * Handles the signing flow for three types of update instructions:
 * 
 *  - Update root keys with root keys
 *  - Update level 1 keys with root keys
 *  - Update level 1 keys with level 1 keys
 * 
 * They are all handled together with a shared function, as their serializations are almost
 * equal.
 * @param cdata please see /doc/ins_higher_level_keys.md
 * @param p1 please see /doc/ins_higher_level_keys.md
 * @param updateType the update type value, should be '10' for updating root keys with root keys, '11' for updating level 1
 * keys with root keys, and '13' for updating level 1 keys with level 1 keys.
 */
void handleSignHigherLevelKeys(uint8_t *cdata, uint8_t p1, uint8_t updateType, volatile unsigned int *flags);
