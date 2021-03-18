#include <os.h>
#include "util.h"
#include "sign.h"

static tx_state_t *tx_state = &global_tx_state;

UX_STEP_CB(
    ux_sign_remove_baker_1_step,
    nn,
    sendSuccessNoIdle(),
    {
      "Remove baker",
      "from pool"
    });
UX_FLOW(ux_sign_remove_baker,
    &ux_sign_flow_shared_review,
    &ux_sign_remove_baker_1_step,
    &ux_sign_flow_shared_sign,
    &ux_sign_flow_shared_decline
);

void handleSignRemoveBaker(uint8_t *cdata, volatile unsigned int *flags) {
    int bytesRead = parseKeyDerivationPath(cdata);
    cdata += bytesRead;

    cx_sha256_init(&tx_state->hash);
    cx_hash((cx_hash_t *) &tx_state->hash, 0, cdata, ACCOUNT_TRANSACTION_HEADER_LENGTH + 1, NULL, 0);
    uint8_t transactionKind = cdata[ACCOUNT_TRANSACTION_HEADER_LENGTH];
    if (transactionKind != REMOVE_BAKER) {
        THROW(SW_INVALID_TRANSACTION);
    }

    ux_flow_init(0, ux_sign_remove_baker, NULL);
    *flags |= IO_ASYNCH_REPLY;
}
