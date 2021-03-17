#include "os.h"
#include "cx.h"
#include "globals.h"

/**
 * Handles the signing flow, including updating the display, for the 'simple transfer'
 * account transaction.
 * @param cdata please see /doc/ins_transfer for details
 */
void handleSignTransfer(uint8_t *dataBuffer, volatile unsigned int *flags);
