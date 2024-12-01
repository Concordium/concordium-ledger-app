#define FUZZ_BUILD
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <sys/types.h>

#include "transaction/deserialize.h"
#include "transaction/utils.h"
#include "transaction/types.h"
#include "format.h"
#include "mocks/ux.h"

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    buffer_t buf = {.ptr = data, .size = size, .offset = 0};
    transaction_ctx_t tx;
    parser_status_e status;
    char address[21] = {0};
    char amount[21] = {0};

    memset(&tx, 0, sizeof(tx));

    status = simple_transfer_deserialize(&buf, &tx);

    if (status == PARSING_OK) {
        // Format recipient address
        format_hex(tx.transaction.simple_transfer.recipient,
                  ADDRESS_LEN,
                  address,
                  sizeof(address));
        printf("recipient: %s\n", address);

        // Format sender address
        format_hex(tx.transaction.simple_transfer.sender,
                  ADDRESS_LEN,
                  address,
                  sizeof(address));
        printf("sender: %s\n", address);

        // Format amount
        format_fpu64(amount,
                    sizeof(amount),
                    tx.transaction.simple_transfer.value,
                    3);  // exponent of smallest unit is 3
        printf("amount: %s\n", amount);
    }

    return 0;
}
