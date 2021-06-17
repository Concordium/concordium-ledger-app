#include <os.h>
#include "util.h"
#include "accountSenderView.h"
#include "sign.h"

static tx_state_t *tx_state = &global_tx_state;

UX_STEP_NOCB(
    ux_sign_remove_baker_1_step,
    nn,
    {
      "Remove baker",
      "from pool"
    });
UX_FLOW(ux_sign_remove_baker,
    &ux_sign_flow_shared_review,
    &ux_sign_flow_account_sender_view,
    &ux_sign_remove_baker_1_step,
    &ux_sign_flow_shared_sign,
    &ux_sign_flow_shared_decline
);

void handleSignRemoveBaker(uint8_t *cdata, volatile unsigned int *flags) {
    cdata += parseKeyDerivationPath(cdata);
    cx_sha256_init(&tx_state->hash);
    cdata += hashAccountTransactionHeaderAndKind(cdata, REMOVE_BAKER);

    // Note that there is no payload in this transaction, as the transaction
    // type itself indicates that the baker for the account should be removed.
    // So that is why it is a little empty here, as everything that has to be
    // processed is in the header.

    ux_flow_init(0, ux_sign_remove_baker, NULL);
    *flags |= IO_ASYNCH_REPLY;
}
