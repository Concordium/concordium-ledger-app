#include <string.h>
#include "getAppName.h"
#include "io.h"
#include "os.h"
#include "globals.h"

void handleGetAppName(volatile unsigned int *flags) {
    // The APPNAME is defined in the Makefile
    uint8_t appName[] = APPNAME;
    uint8_t appNameLength = strlen((char *) appName);

    // Copy app name to response buffer
    memmove(G_io_apdu_buffer, appName, appNameLength);

    // Send response using io_exchange
    io_exchange(CHANNEL_APDU | IO_RETURN_AFTER_TX, appNameLength);
    *flags |= IO_RETURN_AFTER_TX;
}
