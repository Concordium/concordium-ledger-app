#include <os.h>
#include "util.h"
#include "sign.h"

static signTransferToPublic_t *ctx = &global.signTransferToPublic;
static tx_state_t *tx_state = &global_tx_state;

void sendSuccessAndUpdateState(void) {
    ctx->state = TX_TRANSFER_TO_PUBLIC_PROOF;
    sendSuccessNoIdle();
}

UX_STEP_CB(
    ux_sign_transfer_to_public_1_step,
    bn_paging,
    sendSuccessAndUpdateState(),
    {
      .title = "Amount to public",
      .text = (char *) global.signTransferToPublic.amount
    });
UX_FLOW(ux_sign_transfer_to_public,
    &ux_sign_flow_shared_review,
    &ux_sign_transfer_to_public_1_step
);

#define P1_INITIAL          0x00
#define P1_REMAINING_AMOUNT 0x01
#define P1_PROOF            0x02

void handleSignTransferToPublic(uint8_t *cdata, uint8_t p1, uint8_t dataLength, volatile unsigned int *flags) {
    if (p1 == P1_INITIAL) {
        cdata += parseKeyDerivationPath(cdata);
        cx_sha256_init(&tx_state->hash);
        tx_state->initialized = true;
        cdata += hashAccountTransactionHeaderAndKind(cdata, TRANSFER_TO_PUBLIC);

        // Ask the caller for the next command.
        ctx->state = TX_TRANSFER_TO_PUBLIC_REMAINING_AMOUNT;
        sendSuccessNoIdle();
    } else if (p1 == P1_REMAINING_AMOUNT && ctx->state == TX_TRANSFER_TO_PUBLIC_REMAINING_AMOUNT) {
        // Hash remaining amount. Remaining amount is encrypted, and so we cannot display it.
        cx_hash((cx_hash_t *) &tx_state->hash, 0, cdata, 192, NULL, 0);
        cdata += 192;

        // Parse transaction amount so it can be displayed.
        uint64_t amountToPublic = U8BE(cdata, 0);
        amountToGtuDisplay(ctx->amount, amountToPublic);
        cx_hash((cx_hash_t *) &tx_state->hash, 0, cdata, 8, NULL, 0);
        cdata += 8;

        // Hash amount index
        cx_hash((cx_hash_t *) &tx_state->hash, 0, cdata, 8, NULL, 0);
        cdata += 8;

        // Parse size of incoming proofs.
        ctx->proofSize = U2BE(cdata, 0);

        ux_flow_init(0, ux_sign_transfer_to_public, NULL);
        *flags |= IO_ASYNCH_REPLY;
    } else if (p1 == P1_PROOF && ctx->state == TX_TRANSFER_TO_PUBLIC_PROOF) {
        cx_hash((cx_hash_t *) &tx_state->hash, 0, cdata, dataLength, NULL, 0);
        ctx->proofSize -= dataLength;

        if (ctx->proofSize == 0) {
            // We have received all proof bytes, continue to signing flow.
            ux_flow_init(0, ux_sign_flow_shared, NULL);
            *flags |= IO_ASYNCH_REPLY;
        } else if (ctx->proofSize < 0) {
            // We received more proof bytes than expected, and so the received
            // transaction is invalid.
            THROW(SW_INVALID_TRANSACTION);
        } else {
            // There are additional bytes to be received, so ask the caller
            // for more data.
            sendSuccessNoIdle();
        }
    } else {
        THROW(SW_INVALID_STATE);
    }
}
