#include <os.h>
#include <os_io_seproxyhal.h>
#include "cx.h"
#include <stdint.h>
#include "util.h"
#include <stdio.h>
#include "sign.h"

static tx_state_t *tx_state = &global_tx_state;

void buildAndSignTransactionHash();
void declineToSignTransaction();

// Common signature flow for all transactions allowing the user to either sign the transaction hash
// that is currently being processed, or declining to do so (sending back a user rejection error to the computer).
UX_STEP_CB(
    ux_sign_flow_shared_0_step,
    pnn,
    buildAndSignTransactionHash(),
    {
      &C_icon_validate_14,
      "Sign",
      "transaction"
    });
UX_STEP_CB(
    ux_sign_flow_shared_1_step,
    pnn,
    declineToSignTransaction(),
    {
      &C_icon_crossmark,
      "Decline to",
      "sign transaction"
    });
UX_FLOW(ux_sign_flow_shared,
    &ux_sign_flow_shared_0_step,
    &ux_sign_flow_shared_1_step
);

// Hashes transaction, signs it and sends the signature back to the computer.
void buildAndSignTransactionHash() {
    cx_hash((cx_hash_t *) &tx_state->hash, CX_LAST, NULL, 0, tx_state->transactionHash, 32);

    uint8_t signedHash[64];
    signTransactionHash(tx_state->transactionHash, signedHash);

    os_memmove(G_io_apdu_buffer, signedHash, sizeof(signedHash));
    sendSuccess(sizeof(signedHash));

    // Reset initialization status, as we are done processing the current transaction.
    tx_state->initialized = false;
}

// Send user rejection and make sure to reset context (otherwise a new request would be rejected).
void declineToSignTransaction() {
    tx_state->initialized = false;
    sendUserRejection();
}
