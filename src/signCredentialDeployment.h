#include "os.h"
#include "cx.h"
#include "globals.h"

void handleSignCredentialDeployment(uint8_t *dataBuffer, uint8_t p1, uint8_t p2, volatile unsigned int *flags);

void handleSignUpdateCredential(uint8_t *dataBuffer, uint8_t p1, uint8_t p2, volatile unsigned int *flags);