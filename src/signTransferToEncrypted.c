#include <os.h>

#include "accountSenderView.h"
#include "sign.h"
#include "util.h"

static signTransferToEncrypted_t *ctx = &global.signTransferToEncrypted;
static tx_state_t *tx_state = &global_tx_state;

UX_STEP_NOCB(
    ux_sign_transfer_to_encrypted_1_step,
    bnnn_paging,
    {.title = "Shield amount", .text = (char *) global.signTransferToEncrypted.amount});
UX_FLOW(
    ux_sign_transfer_to_encrypted,
    &ux_sign_flow_shared_review,
    &ux_sign_flow_account_sender_view,
    &ux_sign_transfer_to_encrypted_1_step,
    &ux_sign_flow_shared_sign,
    &ux_sign_flow_shared_decline);

void handleSignTransferToEncrypted(uint8_t *cdata, volatile unsigned int *flags) {
    cdata += parseKeyDerivationPath(cdata);
    cx_sha256_init(&tx_state->hash);
    cdata += hashAccountTransactionHeaderAndKind(cdata, TRANSFER_TO_ENCRYPTED);

    // Parse transaction amount so it can be displayed.
    uint64_t amountToEncrypted = U8BE(cdata, 0);
    amountToGtuDisplay(ctx->amount, sizeof(ctx->amount), amountToEncrypted);
    updateHash((cx_hash_t *) &tx_state->hash, cdata, 8);

    ux_flow_init(0, ux_sign_transfer_to_encrypted, NULL);
    *flags |= IO_ASYNCH_REPLY;
}
