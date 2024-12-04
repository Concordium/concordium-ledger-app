#include "sign.h"

#include <os_io_seproxyhal.h>
#include <stdint.h>
#include <stdio.h>

#include "cx.h"
#include "os.h"
#include "util.h"
#include "ux.h"
#include "sign.h"
#include "display.h"

static tx_state_t *tx_state = &global_tx_state;

// Hashes transaction, signs it and sends the signature back to the computer.
void buildAndSignTransactionHash() {
    hash((cx_hash_t *) &tx_state->hash, CX_LAST, NULL, 0, tx_state->transactionHash, 32);

    uint8_t signedHash[64];
    sign(tx_state->transactionHash, signedHash);

    memmove(G_io_apdu_buffer, signedHash, sizeof(signedHash));
    sendSuccess(sizeof(signedHash));
}
