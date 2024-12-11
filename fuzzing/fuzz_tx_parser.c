#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <sys/types.h>
#include "buffer.h"
#include "signTransfer.h"

#define ADDRESS_LEN 20
void format_hex(const uint8_t* data, size_t dataLen, char* dst, size_t dstLen);
void format_fpu64(char* dst, size_t dstLen, uint64_t value, uint8_t decimals);

typedef enum {
    PARSING_OK = 1,
    MEMO_PARSING_ERROR = -1,
    WRONG_LENGTH_ERROR = -7,
    TYPE_PARSING_ERROR = -8,
    SENDER_PARSING_ERROR = -9,
    RECIPIENT_PARSING_ERROR = -10,
    AMOUNT_PARSING_ERROR = -11,
    PARSING_ERROR = -12
} parser_status_e;

// #include "transaction/deserialize.h"
// #include "transaction/utils.h"
// #include "transaction/types.h"
// #include "format.h"

int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
    buffer_t buf = {.ptr = data, .size = size, .offset = 0};
    signTransferContext_t tx;
    parser_status_e status;

    memset(&tx, 0, sizeof(tx));

    volatile unsigned int flags = 0;
    handleSignTransfer((uint8_t*) data, &flags);
    status = (parser_status_e) flags;

    if (status == PARSING_OK) {
        // Print the display string which should contain formatted transaction info
        printf("Display string: %s\n", tx.displayStr);

        // Print the amount
        printf("Display amount: %s\n", tx.displayAmount);

        // Print the state
        printf("State: %d\n", tx.state);
    }

    return 0;
}