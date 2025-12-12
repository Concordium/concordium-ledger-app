#include <assert.h>  // _Static_assert

#include "getAppName.h"
#include "globals.h"

int handleGetAppName() {
    _Static_assert(APPNAME_LEN < MAX_APPNAME_LEN, "APPNAME must be at most 64 characters!");

    return io_send_response_pointer(PIC(APPNAME), APPNAME_LEN, SUCCESS);
}
