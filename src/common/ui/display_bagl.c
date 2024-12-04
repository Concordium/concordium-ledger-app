#ifdef HAVE_BAGL

#include <os.h>

#include "responseCodes.h"
#include "sign.h"
#include "util.h"
#include "display.h"
#include "globals.h"
#include "menu.h"
#include "getPublicKey.h"

accountSender_t global_account_sender;
static cborContext_t *ctx = &global.withDataBlob.cborContext;

UX_STEP_NOCB(ux_display_memo_step_nocb,
             bnnn_paging,
             {"Memo", (char *) global.withDataBlob.cborContext.display});

UX_STEP_CB(ux_display_memo_step,
           bnnn_paging,
           handleCborStep(),
           {"Memo", (char *) global.withDataBlob.cborContext.display});

UX_FLOW(ux_display_memo, &ux_display_memo_step);

UX_STEP_NOCB(ux_sign_flow_account_sender_view,
             bnnn_paging,
             {.title = "Sender", .text = (char *) global_account_sender.sender});

void handleCborStep(void) {
    if (ctx->cborLength < 0) {
        THROW(ERROR_INVALID_STATE);
    } else {
        sendSuccessNoIdle();  // Request more data from the computer.
    }
}

void readCborInitial(uint8_t *cdata, uint8_t dataLength) {
    uint8_t header = cdata[0];
    cdata += 1;
    ctx->cborLength -= 1;
    // the first byte of an cbor encoding contains the type (3 high bits) and the shortCount (5
    // lower bits);
    ctx->majorType = header >> 5;
    uint8_t shortCount = header & 0x1f;

    // Calculate length of cbor payload
    // sizeLength: number of bytes (beside the header) used to indicate the payload length
    uint8_t sizeLength = 0;
    // length: payload byte size.
    uint64_t length = 0;

    ctx->displayUsed = 0;

    if (shortCount < 24) {
        // shortCount is the length, no extra bytes are used.
        sizeLength = 0;
        length = shortCount;
    } else if (shortCount == 24) {
        length = cdata[0];
        sizeLength = 1;
    } else if (shortCount == 25) {
        length = U2BE(cdata, 0);
        sizeLength = 2;
    } else if (shortCount == 26) {
        length = U4BE(cdata, 0);
        sizeLength = 4;
    } else if (shortCount == 27) {
        length = U8BE(cdata, 0);
        sizeLength = 8;
    } else if (shortCount == 31) {
        THROW(ERROR_UNSUPPORTED_CBOR);
    } else {
        THROW(ERROR_INVALID_PARAM);
    }
    cdata += sizeLength;

    ctx->cborLength -= sizeLength;
    switch (ctx->majorType) {
        case 0:
            // non-negative integer
            bin2dec(ctx->display, sizeof(ctx->display), length);
            if (ctx->cborLength != 0) {
                THROW(ERROR_INVALID_STATE);
            }
            break;
        case 1:
            // negative integer
            memmove(ctx->display, "-", 1);
            if (length == UINT64_MAX) {
                bin2dec(ctx->display + 1, sizeof(ctx->display) - 1, length);
                memmove(ctx->display + 1 + 20, " - 1", 4);
            } else {
                bin2dec(ctx->display + 1, sizeof(ctx->display) - 1, 1 + length);
            }
            if (ctx->cborLength != 0) {
                THROW(ERROR_INVALID_STATE);
            }
            break;
        case 3:
            // utf-8 string
            if (ctx->cborLength != length) {
                THROW(ERROR_INVALID_STATE);
            }
            readCborContent(cdata, dataLength - 1 - sizeLength);
            break;
        default:
            THROW(ERROR_UNSUPPORTED_CBOR);
    }
}

void readCborContent(uint8_t *cdata, uint8_t contentLength) {
    ctx->cborLength -= contentLength;
    switch (ctx->majorType) {
        case 3:
            memmove(ctx->display + ctx->displayUsed, cdata, contentLength);
            ctx->displayUsed += contentLength;
            break;
        case 0:
        case 1:
            // Type 0 and 1 fails, because we don't support integers that can't fit in the initial
            // payload.
            THROW(ERROR_UNSUPPORTED_CBOR);
            break;
        default:
            THROW(ERROR_INVALID_STATE);
    }
}

// UI definitions for comparison of public-key on the device
// with the public-key that the caller received.
UX_STEP_NOCB(ux_sign_compare_public_key_0_step,
             bnnn_paging,
             {.title = "Compare", .text = (char *) global.exportPublicKeyContext.publicKey});
UX_STEP_CB(ux_compare_accept_step, pb, ui_menu_main(), {&C_icon_validate_14, "Accept"});
UX_STEP_CB(ux_compare_decline_step, pb, ui_menu_main(), {&C_icon_crossmark, "Decline"});
UX_FLOW(ux_sign_compare_public_key,
        &ux_sign_compare_public_key_0_step,
        &ux_compare_accept_step,
        &ux_compare_decline_step);

void uiComparePubkey(void) {
    ux_flow_init(0, ux_sign_compare_public_key, NULL);
}

UX_STEP_VALID(ux_decline_step, pb, sendUserRejection(), {&C_icon_crossmark, "Decline"});

// UI definitions for the approval of the generation of a public-key. This prompts the user to
// accept that a public-key will be generated and returned to the computer.
UX_STEP_VALID(ux_generate_public_flow_0_step,
              pnn,
              sendPublicKey(true),
              {&C_icon_validate_14, "Public key", (char *) global.exportPublicKeyContext.display});
UX_FLOW(ux_generate_public_flow, &ux_generate_public_flow_0_step, &ux_decline_step, FLOW_LOOP);

void uiGeneratePubkey(volatile unsigned int *flags) {
    // Display the UI for the public-key flow, where the user can validate that the
    // public-key being generated is the expected one.
    ux_flow_init(0, ux_generate_public_flow, NULL);

    // Tell the main process to wait for a button press.
    *flags |= IO_ASYNCH_REPLY;
}
#endif
