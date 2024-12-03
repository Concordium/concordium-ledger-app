#include <os.h>

#include "accountSenderView.h"
#include "displayCbor.h"
#include "responseCodes.h"
#include "sign.h"
#include "util.h"

static signTransferContext_t *ctx = &global.withDataBlob.signTransferContext;
static cborContext_t *memo_ctx = &global.withDataBlob.cborContext;
static tx_state_t *tx_state = &global_tx_state;
const ux_flow_step_t *ux_sign_amount_transfer[8];

UX_STEP_NOCB(
    ux_sign_flow_1_step,
    bnnn_paging,
    {"Amount", (char *) global.withDataBlob.signTransferContext.displayAmount});

UX_STEP_NOCB(
    ux_sign_flow_2_step,
    bnnn_paging,
    {.title = "Recipient", .text = (char *) global.withDataBlob.signTransferContext.displayStr});

void startTransferDisplay(bool displayMemo) {
    uint8_t index = 0;

    ux_sign_amount_transfer[index++] = &ux_sign_flow_shared_review;
    ux_sign_amount_transfer[index++] = &ux_sign_flow_account_sender_view;
    ux_sign_amount_transfer[index++] = &ux_sign_flow_1_step;
    ux_sign_amount_transfer[index++] = &ux_sign_flow_2_step;

    if (displayMemo) {
        ux_sign_amount_transfer[index++] = &ux_display_memo_step_nocb;
    }

    ux_sign_amount_transfer[index++] = &ux_sign_flow_shared_sign;
    ux_sign_amount_transfer[index++] = &ux_sign_flow_shared_decline;

    ux_sign_amount_transfer[index++] = FLOW_END_STEP;
    ux_flow_init(0, ux_sign_amount_transfer, NULL);
}

#define P1_INITIAL           0x00
#define P1_INITIAL_WITH_MEMO 0x01
#define P1_MEMO              0x02
#define P1_AMOUNT            0x03

void handleSignTransfer(uint8_t *cdata, volatile unsigned int *flags) {
    cdata += handleHeaderAndToAddress(cdata, TRANSFER, ctx->displayStr, sizeof(ctx->displayStr));

    // Build display value of the amount to transfer, and also add the bytes to the hash.
    uint64_t amount = U8BE(cdata, 0);
    amountToGtuDisplay(ctx->displayAmount, sizeof(ctx->displayAmount), amount);
    updateHash((cx_hash_t *) &tx_state->hash, cdata, 8);

    // Display the transaction information to the user (recipient address and amount to be sent).
    startTransferDisplay(false);

    // Tell the main process to wait for a button press.
    *flags |= IO_ASYNCH_REPLY;
}

void finishMemo() {
    ctx->state = TX_TRANSFER_AMOUNT;
    sendSuccessNoIdle();
}

void handleSignTransferWithMemo(
    uint8_t *cdata,
    uint8_t p1,
    uint8_t dataLength,
    volatile unsigned int *flags,
    bool isInitialCall) {
    if (isInitialCall) {
        ctx->state = TX_TRANSFER_INITIAL;
    }

    if (p1 == P1_INITIAL_WITH_MEMO && ctx->state == TX_TRANSFER_INITIAL) {
        cdata += handleHeaderAndToAddress(cdata, TRANSFER_WITH_MEMO, ctx->displayStr, sizeof(ctx->displayStr));

        // hash the memo length
        memo_ctx->cborLength = U2BE(cdata, 0);
        if (memo_ctx->cborLength > MAX_MEMO_SIZE) {
            THROW(ERROR_INVALID_PARAM);
        }

        updateHash((cx_hash_t *) &tx_state->hash, cdata, 2);

        ctx->state = TX_TRANSFER_MEMO_INITIAL;
        sendSuccessNoIdle();
    } else if (p1 == P1_MEMO && ctx->state == TX_TRANSFER_MEMO_INITIAL) {
        updateHash((cx_hash_t *) &tx_state->hash, cdata, dataLength);

        readCborInitial(cdata, dataLength);
        if (memo_ctx->cborLength == 0) {
            finishMemo();
        } else {
            ctx->state = TX_TRANSFER_MEMO;
            sendSuccessNoIdle();
        }
    } else if (p1 == P1_MEMO && ctx->state == TX_TRANSFER_MEMO) {
        updateHash((cx_hash_t *) &tx_state->hash, cdata, dataLength);

        readCborContent(cdata, dataLength);
        if (memo_ctx->cborLength != 0) {
            // The memo size is <=256 bytes, so we should always have received the complete memo by this point
            THROW(ERROR_INVALID_STATE);
        }

        finishMemo();
    } else if (p1 == P1_AMOUNT && ctx->state == TX_TRANSFER_AMOUNT) {
        // Build display value of the amount to transfer, and also add the bytes to the hash.
        uint64_t amount = U8BE(cdata, 0);
        amountToGtuDisplay(ctx->displayAmount, sizeof(ctx->displayAmount), amount);
        updateHash((cx_hash_t *) &tx_state->hash, cdata, 8);

        startTransferDisplay(true);
        *flags |= IO_ASYNCH_REPLY;
    } else {
        THROW(ERROR_INVALID_STATE);
    }
}
