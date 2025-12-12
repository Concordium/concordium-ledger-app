#include "globals.h"

// CBOR encoding constants
#define CBOR_SHORT_COUNT_MAX   23  // Values below this are direct length
#define CBOR_ONE_BYTE_LENGTH   24  // Uses 1 additional byte for length
#define CBOR_TWO_BYTE_LENGTH   25  // Uses 2 additional bytes for length
#define CBOR_FOUR_BYTE_LENGTH  26  // Uses 4 additional bytes for length
#define CBOR_EIGHT_BYTE_LENGTH 27  // Uses 8 additional bytes for length
#define CBOR_INDEFINITE_LENGTH 31  // Indicates indefinite length encoding (unsupported)

// Mask and bit shifts
#define CBOR_SHORT_COUNT_MASK 0x1F  // 5 lower bits

static tx_state_t *tx_state = &global_tx_state;
static cborContext_t *ctx = &global.withDataBlob.cborContext;

// Hashes transaction, signs it and sends the signature back to the computer.
void buildAndSignTransactionHash() {
    hash((cx_hash_t *)&tx_state->hash, CX_LAST, NULL, 0, tx_state->transactionHash, 32);

    uint8_t signedHash[COMMON_SIGNATURE_SIZE];
    sign(tx_state->transactionHash, signedHash);
    if (sizeof(signedHash) > sizeof(G_io_apdu_buffer)) {
        THROW(ERROR_BUFFER_OVERFLOW);
    }
    memmove(G_io_apdu_buffer, signedHash, sizeof(signedHash));
    sendSuccess(sizeof(signedHash));
}

void readCborInitial(uint8_t *cdata, uint8_t dataLength) {
    uint8_t remainingDataLength = dataLength;
    if (remainingDataLength < 1) {
        THROW(ERROR_BUFFER_OVERFLOW);
    }
    uint8_t header = cdata[0];
    cdata += 1;
    remainingDataLength -= 1;
    ctx->cborLength -= 1;
    // the first byte of an cbor encoding contains the type (3 high bits) and the shortCount (5
    // lower bits);
    ctx->majorType = header >> 5;
    uint8_t shortCount = header & CBOR_SHORT_COUNT_MASK;

    // Calculate length of cbor payload
    // sizeLength: number of bytes (beside the header) used to indicate the payload length
    uint8_t sizeLength = 0;
    // length: payload byte size.
    uint64_t length = 0;

    ctx->displayUsed = 0;

    if (shortCount <= CBOR_SHORT_COUNT_MAX) {
        // shortCount is the length, no extra bytes are used.
        length = shortCount;
    } else if (shortCount == CBOR_ONE_BYTE_LENGTH) {
        if (remainingDataLength < 1) {
            THROW(ERROR_BUFFER_OVERFLOW);
        }
        length = cdata[0];
        sizeLength = 1;
    } else if (shortCount == CBOR_TWO_BYTE_LENGTH) {
        if (remainingDataLength < 2) {
            THROW(ERROR_BUFFER_OVERFLOW);
        }
        length = U2BE(cdata, 0);
        sizeLength = 2;
    } else if (shortCount == CBOR_FOUR_BYTE_LENGTH) {
        if (remainingDataLength < 4) {
            THROW(ERROR_BUFFER_OVERFLOW);
        }
        length = U4BE(cdata, 0);
        sizeLength = 4;
    } else if (shortCount == CBOR_EIGHT_BYTE_LENGTH) {
        if (remainingDataLength < 8) {
            THROW(ERROR_BUFFER_OVERFLOW);
        }
        length = U8BE(cdata, 0);
        sizeLength = 8;
    } else if (shortCount == CBOR_INDEFINITE_LENGTH) {
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