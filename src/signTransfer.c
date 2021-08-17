#include <os.h>
#include "util.h"
#include "sign.h"
#include "accountSenderView.h"
#include "base58check.h"
#include "responseCodes.h"

static signTransferContext_t *ctx = &global.signTransferContext;
static tx_state_t *tx_state = &global_tx_state;

void handleMemoStep();

UX_STEP_NOCB(
    ux_sign_flow_1_step,
    bnnn_paging,
    {
      "Amount (GTU)",
      (char *) global.signTransferContext.displayAmount,
    });

UX_STEP_NOCB(
    ux_sign_flow_2_step,
    bnnn_paging,
    {
      .title = "Recipient",
      .text = (char *) global.signTransferContext.displayStr
    });
UX_FLOW(ux_sign_flow,
    &ux_sign_flow_shared_review,
    &ux_sign_flow_account_sender_view,
    &ux_sign_flow_1_step,
    &ux_sign_flow_2_step,
    &ux_sign_flow_shared_sign,
    &ux_sign_flow_shared_decline
);

UX_STEP_CB(
    ux_sign_flow_2_step_cb,
    bnnn_paging,
    sendSuccessNoIdle(),
    {
        .title = "Recipient",
            .text = (char *) global.signTransferContext.displayStr
            });

UX_FLOW(ux_sign_flow_memo,
        &ux_sign_flow_shared_review,
        &ux_sign_flow_account_sender_view,
        &ux_sign_flow_1_step,
        &ux_sign_flow_2_step_cb
    );

UX_STEP_CB(
    ux_sign_transfer_memo_step,
    bnnn_paging,
    handleMemoStep(),
    {
        "Memo",
            (char *) global.signTransferContext.memo
            });
UX_FLOW(ux_sign_transfer_memo,
        &ux_sign_transfer_memo_step
    );

void handleMemoStep() {
    if (ctx->memoLength == 0) {
        ux_flow_init(0, ux_sign_flow_shared, NULL);
    } else if (ctx->memoLength > 0) {
        sendSuccessNoIdle();   // Request more data from the computer.
    } else {
        THROW(ERROR_INVALID_STATE);
    }
}

#define P1_INITIAL          0x00
#define P1_MEMO            0x01

void handleSignTransfer(uint8_t *cdata, uint8_t p1, uint8_t dataLength, volatile unsigned int *flags, bool isInitialCall) {
    if (isInitialCall) {
        ctx->state = TX_TRANSFER_INITIAL;
    }

    if (p1 == P1_INITIAL && ctx->state == TX_TRANSFER_INITIAL) {
    // Parse the key derivation path, which should always be the first thing received
    // in a command to the Ledger application.
    cdata += parseKeyDerivationPath(cdata);

    uint8_t withMemo = cdata[0];

    cdata += 1;

    // Initialize the hash that will be the hash of the whole transaction, which is what will be signed
    // if the user approves.
    cx_sha256_init(&tx_state->hash);
    cdata += hashAccountTransactionHeaderAndKind(cdata, TRANSFER);

    // Extract the recipient address and add to the hash.
    uint8_t toAddress[32];
    memmove(toAddress, cdata, 32);
    cdata += 32;
    cx_hash((cx_hash_t *) &tx_state->hash, 0, toAddress, 32, NULL, 0);

    // The recipient address is in a base58 format, so we need to encode it to be
    // able to display in a humand-readable way. This is written to ctx->displayStr as a string
    // so that it can be displayed.
    size_t outputSize = sizeof(ctx->displayStr);
    if (base58check_encode(toAddress, sizeof(toAddress), ctx->displayStr, &outputSize) != 0) {
      // The received address bytes are not a valid base58 encoding.
        THROW(ERROR_INVALID_TRANSACTION);
    }
    ctx->displayStr[50] = '\0';

    // Build display value of the amount to transfer, and also add the bytes to the hash.
    uint64_t amount = U8BE(cdata, 0);
    amountToGtuDisplay(ctx->displayAmount, amount);
    cx_hash((cx_hash_t *) &tx_state->hash, 0, cdata, 8, NULL, 0);

    // Display the transaction information to the user (recipient address and amount to be sent).
    if (withMemo == 0) {
        ux_flow_init(0, ux_sign_flow, NULL);
    } else {
        ctx->state = TX_TRANSFER_MEMO_LENGTH;
        ux_flow_init(0, ux_sign_flow_memo, NULL);
    }
    // Tell the main process to wait for a button press.
    *flags |= IO_ASYNCH_REPLY;
    } else if (p1 == P1_MEMO && ctx->state == TX_TRANSFER_MEMO_LENGTH) {
        cx_hash((cx_hash_t *) &tx_state->hash, 0, cdata, dataLength, NULL, 0);
        uint8_t header = cdata[0];
        cdata+=1;
        uint8_t majorType = header >> 5;
        uint8_t shortCount = header & 0x1f;

        uint8_t cLength;
        uint64_t length;

        if (shortCount < 24) {
            cLength = 0;
            length = shortCount;
        } else if (shortCount == 24) {
            length = cdata[0];
            cLength= 1;
        } else if (shortCount == 25) {
            length = U2BE(cdata,0);
            cLength = 2;
        } else if (shortCount == 26) {
            length = U4BE(cdata,0);
            cLength = 4;
        } else if (shortCount == 27) {
            length = U8BE(cdata,0);
            cLength = 8;
        } else {
            THROW(ERROR_INVALID_STATE);
        }

        cdata += cLength;

        if (majorType == 0) {
            bin2dec(ctx->memo, length);
            ctx->memoLength = 0;
        } else if (majorType == 1) {
            bin2dec(ctx->memo, 1 - length);
            ctx->memoLength = 0;
        } else if (majorType == 2 || majorType == 3) {
            // string
            uint8_t stringLength = dataLength - 1 - cLength;
            ctx->memoLength = length;
            memmove(ctx->memo, cdata, stringLength);
            ctx->memoLength -= stringLength;

            if (stringLength < 255) {
                memmove(ctx->memo + stringLength, "\0", 1);
            }
        } else {
            // Change to unsupported cbor encoding;
            THROW(ERROR_INVALID_STATE);
        }

        ctx->state = TX_TRANSFER_MEMO;
        ux_flow_init(0, ux_sign_transfer_memo, NULL);
        *flags |= IO_ASYNCH_REPLY;
    } else if (p1 == P1_MEMO && ctx->state == TX_TRANSFER_MEMO) {
        cx_hash((cx_hash_t *) &tx_state->hash, 0, cdata, dataLength, NULL, 0);
        memmove(ctx->memo, cdata, dataLength);
        ctx->memoLength -= dataLength;

        if (dataLength < 255) {
            memmove(ctx->memo + dataLength, "\0", 1);
        }

        if (ctx->memoLength < 0) {
            // We received more bytes than expected.
            THROW(ERROR_INVALID_STATE);
            return;
        }

        ux_flow_init(0, ux_sign_transfer_memo, NULL);
        *flags |= IO_ASYNCH_REPLY;
    } else {
        THROW(ERROR_INVALID_STATE);
    }
}
