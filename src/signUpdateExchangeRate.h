#ifndef _CONCORDIUM_APP_UPDATE_EXCHANGE_RATE_H_
#define _CONCORDIUM_APP_UPDATE_EXCHANGE_RATE_H_

/**
 * Handles the signing flow, including updating the display, for two different update types
 * that have identical payloads, namely the 'update micro gtu per euro' and 'update euro per energy'
 * update instructions. The display is automatically adjusted to the correct update type based
 * on the transaction received, as it can be derived from the update type byte.
 * @param cdata please see /doc/ins_update_exchange_rate.md for details
 */
void handleSignUpdateExchangeRate(uint8_t *cdata, volatile unsigned int *flags);

#endif
