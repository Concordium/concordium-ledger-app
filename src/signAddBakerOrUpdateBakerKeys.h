/**
 * Method for signing either an 'AddBaker' transaction or an 'UpdateBakerKeys' transaction. The method
 * is shared as the latter transaction is almost a subset of the prior (except a difference in transaction kind byte).
 */
void handleSignAddBakerOrUpdateBakerKeys(uint8_t *dataBuffer, uint8_t p1, uint8_t p2, volatile unsigned int *flags);
