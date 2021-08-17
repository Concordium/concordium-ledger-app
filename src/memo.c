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

void readMemoInitial(uint8_t *cdata, uint8_t dataLength) {
    uint8_t header = cdata[0];
    cdata+=1;
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
        // TODO: support 'infinite' length strings?
        THROW(ERROR_UNSUPPORTED_CBOR);
    }
    cdata += sizeLength;


    switch (ctx->majorType) {
    case 0:
        // non-negative integer
        bin2dec(ctx->memo, length);
        ctx->memoLength = 0;
        break;
    case 1:
        // negative integer
        bin2dec(ctx->memo, 1 - length);
        ctx->memoLength = 0;
        break;
    case 2:
        // byte string
    case 3:
        // utf-8 string
        ctx->memoLength = length;
        readMemoContent(cdata, dataLength - 1 - sizeLength);
        break;
    case 4:
        // array
    case 5:
        // maps
    case 6:
        // Tag
    case 7:
        // Change to 'unsupported cbor encoding;
        THROW(ERROR_UNSUPPORTED_CBOR);
        break;
    default:
        THROW(ERROR_INVALID_STATE);
    }
}

void readMemoContent(uint8_t *cdata, uint8_t dataLength) {
    switch (ctx->majorType) {
    case 2:
        toHex(cdata, dataLength, ctx->memo);
        if (dataLength * 2 < 255) {
            memmove(ctx->memo + dataLength * 2, "\0", 1);
        }
        break;
    case 3:
        memmove(ctx->memo, cdata, dataLength);
        ctx->memoLength -= dataLength;

        if (dataLength < 255) {
            memmove(ctx->memo + dataLength, "\0", 1);
        }
        break;
    case 0:
    case 1:
        // case 0 and 1  fails, because we don't support integers that can't fit in the initial payload.
    case 4:
    case 5:
    case 6:
    case 7:
        // Change to unsupported cbor encoding;
        THROW(ERROR_UNSUPPORTED_CBOR);
        break;
    default:
        THROW(ERROR_INVALID_STATE);
    }
}
