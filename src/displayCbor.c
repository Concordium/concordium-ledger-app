#include <os.h>

#include "responseCodes.h"
#include "sign.h"
#include "util.h"

static cborContext_t *ctx = &global.withDataBlob.cborContext;

void handleCborStep(void);

UX_STEP_NOCB(ux_display_memo_step_nocb, bnnn_paging, {"Memo", (char *) global.withDataBlob.cborContext.display});

UX_STEP_CB(
    ux_display_memo_step,
    bnnn_paging,
    handleCborStep(),
    {"Memo", (char *) global.withDataBlob.cborContext.display});

UX_FLOW(ux_display_memo, &ux_display_memo_step);

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
    // the first byte of an cbor encoding contains the type (3 high bits) and the shortCount (5 lower bits);
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
            // Type 0 and 1 fails, because we don't support integers that can't fit in the initial payload.
            THROW(ERROR_UNSUPPORTED_CBOR);
            break;
        default:
            THROW(ERROR_INVALID_STATE);
    }
}
