#include <os.h>

#include "base58check.h"
#include "responseCodes.h"
#include "sign.h"
#include "util.h"

static signMessageContext_t *ctx = &global.signMessageContext;
static tx_state_t *tx_state = &global_tx_state;

// Common initial view for signing flows.
UX_STEP_NOCB(ux_sign_message_review, nn, {"Review", "Message request"});

UX_STEP_NOCB(
    ux_sign_message_account_view,
    bnnn_paging,
    {.title = "Signer", .text = (char *) global.signMessageContext.displaySigner});

UX_STEP_NOCB(
    ux_sign_message_display_message,
    bnnn_paging,
    {"Message", (char *) global.signMessageContext.display});

UX_STEP_CB(
    ux_sign_message_accept,
    pnn,
    buildAndSignTransactionHash(),
    {&C_icon_validate_14, "Sign", "Message"});

UX_STEP_CB(
    ux_sign_message_decline,
    pnn,
    sendUserRejection(),
    {&C_icon_crossmark, "Decline to", "sign message"});

// There will at most be 6 UI steps when the entire message fits in one batch.
const ux_flow_step_t *ux_sign_message[6];

void startSignMessageDisplay(bool displayStart, bool finalChunk) {
    uint8_t index = 0;

    if (displayStart) {
            ux_sign_message[index++] = &ux_sign_message_review;
            ux_sign_message[index++] = &ux_sign_message_account_view;
    }

    ux_sign_message[index++] = &ux_sign_message_display_message;

    if (finalChunk) {
        ux_sign_message[index++] = &ux_sign_message_accept;
        ux_sign_message[index++] = &ux_sign_message_decline;
    }

    ux_sign_message[index++] = FLOW_END_STEP;

    ux_flow_init(0, ux_sign_message, NULL);
}


#define P1_SIGN_MESSAGE_INITIAL           0x00
#define P1_SIGN_MESSAGE_MESSAGE      0x01

void handleSignMessage(
    uint8_t *cdata,
    uint8_t p1,
    uint8_t dataLength,
    volatile unsigned int *flags,
    bool isInitialCall) {

    if (isInitialCall) {
        ctx->state = SIGN_MESSAGE_INITIAL;
    }

    if (p1 == P1_SIGN_MESSAGE_INITIAL && ctx->state == SIGN_MESSAGE_INITIAL) {
        cdata += parseKeyDerivationPath(cdata);
        cx_sha256_init(&tx_state->hash);

        uint8_t toAddress[32];
        memmove(toAddress, cdata, 32);
        cx_hash((cx_hash_t *) &tx_state->hash, 0, toAddress, 32, NULL, 0);
        size_t displaySignerSize = sizeof(ctx->displaySigner);
        if (base58check_encode(toAddress, sizeof(toAddress), ctx->displaySigner, &displaySignerSize) != 0) {
            THROW(ERROR_INVALID_TRANSACTION);
        }
        ctx->displaySigner[55] = '\0';
        cdata += 32;
        ctx->messageLength = U2BE(cdata, 0);
        ctx->displayStart = true;

        // We hash 8 zero bytes
        uint8_t zeroes[8];
        memset(zeroes, 0, 8);
        cx_hash((cx_hash_t *) &tx_state->hash, 0, zeroes, 8, NULL, 0);
        ctx->state = SIGN_MESSAGE_CONTINUED;

        sendSuccessNoIdle();
    } else if (p1 == P1_SIGN_MESSAGE_MESSAGE && ctx->state == SIGN_MESSAGE_CONTINUED) {
        cx_hash((cx_hash_t *) &tx_state->hash, 0, cdata, dataLength, NULL, 0);
        ctx->messageLength -= dataLength;
        memmove(ctx->display, cdata, dataLength);
        if (dataLength < 255) {
            memmove(ctx->display + dataLength, "\0", 1);
        }
        startSignMessageDisplay(ctx->displayStart, ctx->messageLength == 0);
        ctx->displayStart = false;
        *flags |= IO_ASYNCH_REPLY;
   } else {
        THROW(ERROR_INVALID_STATE);
    }
}
