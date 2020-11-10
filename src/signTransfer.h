#include "os.h"
#include "cx.h"
#include "globals.h"

void handleSignTransfer(uint8_t *dataBuffer, uint16_t dataLength, volatile unsigned int *flags);

void signTransferHash(uint8_t *transactionHash);

void buildTransferHash(uint8_t *transactionHash, uint8_t *dataBuffer);