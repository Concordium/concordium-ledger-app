#include <string.h>
#include "getAppName.h"
#include "io.h"
#include "os.h"
#include "globals.h"
#include "common/responseCodes.h"
#include "constants.h"

int handleGetAppName() {
    _Static_assert(APPNAME_LEN < MAX_APPNAME_LEN, "APPNAME must be at most 64 characters!");

    return io_send_response_pointer(PIC(APPNAME), APPNAME_LEN, SUCCESS);
}
