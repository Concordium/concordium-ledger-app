#ifndef _CONCORDIUM_APP_ACCOUNT_TRANSFER_WITH_SCHEDULE_H_
#define _CONCORDIUM_APP_ACCOUNT_TRANSFER_WITH_SCHEDULE_H_

/**
 * Handles the signing flow for the transfer with schedule account transaction.
 * @param cdata please see /doc/ins_transfer_with_schedule.md for details
 * @param p1 0x00 for the initial packet containing key derivation path, account transaction header,
 * transaction kind, recipient address and the number of scheduled transfers to make, 0x01 when
 * sending pairs of scheduled amounts.
 */ 
void handleSignTransferWithSchedule(uint8_t *dataBuffer, uint8_t p1, volatile unsigned int *flags);

#endif
