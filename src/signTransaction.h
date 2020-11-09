#include "os.h"
#include "cx.h"
#include "globals.h"

void handleSignTransaction(uint8_t p1, uint8_t *dataBuffer, uint16_t dataLength, volatile unsigned int *flags);

void signTransactionHash(uint8_t *transactionHash);

void buildTransactionHash(uint8_t *transactionHash, uint8_t *dataBuffer);