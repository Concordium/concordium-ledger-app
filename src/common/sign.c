#include "sign.h"

#include <os_io_seproxyhal.h>
#include <stdint.h>
#include <stdio.h>

#include "cx.h"
#include "os.h"
#include "util.h"
#include "ux.h"

static tx_state_t *tx_state = &global_tx_state;

void buildAndSignTransactionHash();

// Common initial view for signing flows.
UX_STEP_NOCB(ux_sign_flow_shared_review, nn, {"Review", "transaction"});

// Common signature flow for all transactions allowing the user to either sign the transaction hash
// that is currently being processed, or declining to do so (sending back a user rejection error to the caller).
UX_STEP_CB(ux_sign_flow_shared_sign, pnn, buildAndSignTransactionHash(), {&C_icon_validate_14, "Sign", "transaction"});
UX_STEP_CB(
    ux_sign_flow_shared_decline,
    pnn,
    sendUserRejection(),
    {&C_icon_crossmark, "Decline to", "sign transaction"});
UX_FLOW(ux_sign_flow_shared, &ux_sign_flow_shared_sign, &ux_sign_flow_shared_decline);

// Hashes transaction, signs it and sends the signature back to the computer.
void buildAndSignTransactionHash() {
    hash((cx_hash_t *) &tx_state->hash, CX_LAST, NULL, 0, tx_state->transactionHash, 32);

    uint8_t signedHash[64];
    sign(tx_state->transactionHash, signedHash);

    memmove(G_io_apdu_buffer, signedHash, sizeof(signedHash));
    sendSuccess(sizeof(signedHash));
}
