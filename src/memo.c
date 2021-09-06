#include <os.h>
#include "util.h"
#include "sign.h"
#include "responseCodes.h"

static memoContext_t *ctx = &global.withMemo.memoContext;
static tx_state_t *tx_state = &global_tx_state;

void handleMemoStep();

UX_STEP_CB(
    ux_sign_transfer_memo_step,
    bnnn_paging,
    handleMemoStep(),
    {
        "Memo",
            (char *) global.withMemo.memoContext.memo
            });

UX_STEP_CB(
    ux_sign_transfer_memo_raw_step,
    bnnn_paging,
    handleMemoStep(),
    {
        "Memo (raw)",
            (char *) global.withMemo.memoContext.memo
            });
UX_FLOW(ux_sign_transfer_memo,
        &ux_sign_transfer_memo_step
    );
UX_FLOW(ux_sign_transfer_memo_raw,
        &ux_sign_transfer_memo_raw_step
    );

void handleMemoStep() {
    if (ctx->memoLength < 0) {
        THROW(ERROR_INVALID_STATE);
    } else {
        sendSuccessNoIdle();   // Request more data from the computer.
    }
}

void readMemoInitial(uint8_t *cdata, uint8_t dataLength) {
    uint8_t header = cdata[0];
    cdata+=1;
    ctx->memoLength -= 1;
    // the first byte of an cbor encoding contains the type (3 high bits) and the shortCount (5 lower bits);
    ctx->majorType = header >> 5;
    uint8_t shortCount = header & 0x1f;

    // Calculate length of cbor payload
    // sizeLength: number of bytes (beside the header) used to indicate the payload length
    uint8_t sizeLength = 0;
    // length: payload byte size.
    uint64_t length = 0;

    if (ctx->majorType > 7 || (shortCount > 27 && shortCount < 31)) {
        // Payload doesn't fit cbor encoding.
        THROW(ERROR_INVALID_STATE);
    } else if (shortCount < 24) {
        // shortCount is the length, no extra bytes is used.
        sizeLength = 0;
        length = shortCount;
    } else if (shortCount == 24) {
        length = cdata[0];
        sizeLength= 1;
    } else if (shortCount == 25) {
        length = U2BE(cdata,0);
        sizeLength = 2;
    } else if (shortCount == 26) {
        length = U4BE(cdata,0);
        sizeLength = 4;
    } else if (shortCount == 27) {
        length = U8BE(cdata,0);
        sizeLength = 8;
    } else if (shortCount == 31) {
        // TODO: support indefinite length strings / streaming?
        THROW(ERROR_UNSUPPORTED_CBOR);
    }
    cdata += sizeLength;
    ctx->memoLength -= sizeLength;

    switch (ctx->majorType) {
    case 0:
        // non-negative integer
        bin2dec(ctx->memo, length);
        if (ctx->memoLength != 0) {
            THROW(ERROR_INVALID_STATE);
        }
        break;
    case 1:
        // negative integer
        bin2dec(ctx->memo, 1 - length);
        if (ctx->memoLength != 0) {
            THROW(ERROR_INVALID_STATE);
        }
        break;
    case 2:
        // byte string
    case 3:
        // utf-8 string
    case 4:
        // array
    case 5:
        // maps
    case 6:
        // Tag
    case 7:
        // special values or break marker
        if (ctx->memoLength != length) {
            THROW(ERROR_INVALID_STATE);
        }
        readMemoContent(cdata, dataLength - 1 - sizeLength);
        break;
    default:
        THROW(ERROR_INVALID_STATE);
    }
}

void readMemoContent(uint8_t *cdata, uint8_t dataLength) {
    ctx->memoLength -= dataLength;
    switch (ctx->majorType) {
    case 3:
        memmove(ctx->memo, cdata, dataLength);

        if (dataLength < 255) {
            memmove(ctx->memo + dataLength, "\0", 1);
        }
        break;
    case 4:
    case 5:
    case 6:
    case 7:
    case 2:
        // Type 2 is a byte string, so we show that as hex. For type 4-7, we display as hex instead of decoding.
        toHex(cdata, dataLength, ctx->memo);
        if (dataLength * 2 < 255) {
            memmove(ctx->memo + dataLength * 2, "\0", 1);
        }
        break;
    case 0:
    case 1:
        // Type 0 and 1 fails, because we don't support integers that can't fit in the initial payload.
        // Change to unsupported cbor encoding;
        THROW(ERROR_UNSUPPORTED_CBOR);
        break;
    default:
        THROW(ERROR_INVALID_STATE);
    }
}

void displayMemo(volatile unsigned int *flags) {
    switch (ctx->majorType) {
    case 0:
    case 1:
    case 2:
    case 3:
        ux_flow_init(0, ux_sign_transfer_memo, NULL);
        *flags |= IO_ASYNCH_REPLY;
        break;
    case 4:
    case 5:
    case 6:
    case 7:
        ux_flow_init(0, ux_sign_transfer_memo_raw, NULL);
        *flags |= IO_ASYNCH_REPLY;
        break;
    default:
        THROW(ERROR_INVALID_STATE);
    }
}
