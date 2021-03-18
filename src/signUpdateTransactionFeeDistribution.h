#ifndef _CONCORDIUM_APP_UPDATE_TRANSACTION_FEE_DISTRIBUTION_H_
#define _CONCORDIUM_APP_UPDATE_TRANSACTION_FEE_DISTRIBUTION_H_

/**
 * Handles the signing flow, including updating the display, for the 'update transaction fee distribution'
 * update instruction.
 * @param cdata please see /doc/ins_update_transaction_fee_distribution.md for details
 */
void handleSignUpdateTransactionFeeDistribution(uint8_t *dataBuffer, volatile unsigned int *flags);

#endif
