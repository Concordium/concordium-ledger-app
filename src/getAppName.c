#include <string.h>
#include "getAppName.h"
#include "io.h"
#include "os.h"
#include "globals.h"
#include "responseCodes.h"

void handleGetAppName(volatile unsigned int *flags) {
    // The APPNAME is defined in the Makefile
    uint8_t appName[] = APPNAME;
    uint8_t appNameLength = strlen((char *) appName);

    // Copy app name to response buffer
    memmove(G_io_apdu_buffer, appName, appNameLength);

    // Add success status word at the end
    G_io_apdu_buffer[appNameLength] = SUCCESS >> 8;
    G_io_apdu_buffer[appNameLength + 1] = SUCCESS & 0xFF;

    // Send response using io_exchange with total length including status word
    io_exchange(CHANNEL_APDU | IO_RETURN_AFTER_TX, appNameLength + 2);
    *flags |= IO_RETURN_AFTER_TX;
}
