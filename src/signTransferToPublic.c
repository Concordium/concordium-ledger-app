#include <os.h>

#include "accountSenderView.h"
#include "responseCodes.h"
#include "sign.h"
#include "util.h"

static signTransferToPublic_t *ctx = &global.signTransferToPublic;
static tx_state_t *tx_state = &global_tx_state;

UX_STEP_NOCB(
    ux_sign_transfer_to_public_1_step,
    bnnn_paging,
    {.title = "Unshield amount", .text = (char *) global.signTransferToPublic.amount});
UX_FLOW(
    ux_sign_transfer_to_public,
    &ux_sign_flow_shared_review,
    &ux_sign_flow_account_sender_view,
    &ux_sign_transfer_to_public_1_step,
    &ux_sign_flow_shared_sign,
    &ux_sign_flow_shared_decline);

#define P1_INITIAL          0x00
#define P1_REMAINING_AMOUNT 0x01
#define P1_PROOF            0x02

void handleSignTransferToPublic(
    uint8_t *cdata,
    uint8_t p1,
    uint8_t dataLength,
    volatile unsigned int *flags,
    bool isInitialCall) {
    if (isInitialCall) {
        ctx->state = TX_TRANSFER_TO_PUBLIC_INITIAL;
    }

    if (p1 == P1_INITIAL && ctx->state == TX_TRANSFER_TO_PUBLIC_INITIAL) {
        cdata += parseKeyDerivationPath(cdata);
        cx_sha256_init(&tx_state->hash);
        hashAccountTransactionHeaderAndKind(cdata, TRANSFER_TO_PUBLIC);

        ctx->state = TX_TRANSFER_TO_PUBLIC_REMAINING_AMOUNT;
        // Ask the caller for the next command.
        sendSuccessNoIdle();
    } else if (p1 == P1_REMAINING_AMOUNT && ctx->state == TX_TRANSFER_TO_PUBLIC_REMAINING_AMOUNT) {
        // Hash remaining amount. Remaining amount is encrypted, and so we cannot display it.
        updateHash((cx_hash_t *) &tx_state->hash, cdata, 192);
        cdata += 192;

        // Parse transaction amount so it can be displayed.
        uint64_t amountToPublic = U8BE(cdata, 0);
        amountToGtuDisplay(ctx->amount, sizeof(ctx->amount), amountToPublic);
        updateHash((cx_hash_t *) &tx_state->hash, cdata, 8);
        cdata += 8;

        // Hash amount index
        updateHash((cx_hash_t *) &tx_state->hash, cdata, 8);
        cdata += 8;

        // Parse size of incoming proofs.
        ctx->proofSize = U2BE(cdata, 0);

        ctx->state = TX_TRANSFER_TO_PUBLIC_PROOF;
        sendSuccessNoIdle();
    } else if (p1 == P1_PROOF && ctx->state == TX_TRANSFER_TO_PUBLIC_PROOF) {
        updateHash((cx_hash_t *) &tx_state->hash, cdata, dataLength);
        ctx->proofSize -= dataLength;

        if (ctx->proofSize == 0) {
            // We have received all proof bytes, continue to signing flow.
            ux_flow_init(0, ux_sign_transfer_to_public, NULL);
            *flags |= IO_ASYNCH_REPLY;
        } else if (ctx->proofSize < 0) {
            // We received more proof bytes than expected, and so the received
            // transaction is invalid.
            THROW(ERROR_INVALID_TRANSACTION);
        } else {
            // There are additional bytes to be received, so ask the caller
            // for more data.
            sendSuccessNoIdle();
        }
    } else {
        THROW(ERROR_INVALID_STATE);
    }
}
