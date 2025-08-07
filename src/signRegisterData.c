#include "globals.h"

static signRegisterData_t *ctx = &global.withDataBlob.signRegisterData;
static cborContext_t *data_ctx = &global.withDataBlob.cborContext;
static tx_state_t *tx_state = &global_tx_state;

void handleData() {
    if (ctx->dataLength > 0) {
        sendSuccessNoIdle();
    } else {
        uiSignFlowSharedDisplay();
    }
}

#define P1_INITIAL 0x00
#define P1_DATA    0x01

void handleSignRegisterData(uint8_t *cdata,
                            uint8_t p1,
                            uint8_t dataLength,
                            volatile unsigned int *flags,
                            bool isInitialCall) {
    if (isInitialCall) {
        ctx->state = TX_REGISTER_DATA_INITIAL;
    }
    uint8_t remainingDataLength = dataLength;
    if (p1 == P1_INITIAL && ctx->state == TX_REGISTER_DATA_INITIAL) {
        size_t offset = parseKeyDerivationPath(cdata, remainingDataLength);
        if (offset > dataLength) {
            THROW(ERROR_BUFFER_OVERFLOW);  // Ensure safe access
        }
        cdata += offset;
        remainingDataLength -= offset;
        if (cx_sha256_init(&tx_state->hash) != CX_SHA256) {
            THROW(ERROR_FAILED_CX_OPERATION);
        }

        offset = hashAccountTransactionHeaderAndKind(cdata, remainingDataLength, REGISTER_DATA);
        if (offset > dataLength) {
            THROW(ERROR_BUFFER_OVERFLOW);  // Ensure safe access
        }
        cdata += offset;
        remainingDataLength -= offset;
        // hash the data length
        if (remainingDataLength < 2) {
            THROW(ERROR_BUFFER_OVERFLOW);
        }
        ctx->dataLength = U2BE(cdata, 0);
        if (ctx->dataLength > MAX_DATA_SIZE) {
            THROW(ERROR_INVALID_PARAM);
        }
        data_ctx->cborLength = ctx->dataLength;
        updateHash((cx_hash_t *)&tx_state->hash, cdata, 2);

        ctx->state = TX_REGISTER_DATA_PAYLOAD_START;

        uiRegisterDataInitialDisplay(flags);

    } else if (p1 == P1_DATA) {
        if (ctx->dataLength < dataLength) {
            THROW(ERROR_INVALID_TRANSACTION);
        }
        ctx->dataLength -= dataLength;
        updateHash((cx_hash_t *)&tx_state->hash, cdata, dataLength);

        switch (ctx->state) {
            case TX_REGISTER_DATA_PAYLOAD_START:
                ctx->state = TX_REGISTER_DATA_PAYLOAD;
                readCborInitial(cdata, dataLength);
                break;
            case TX_REGISTER_DATA_PAYLOAD:
                if (ctx->dataLength != 0) {
                    // The data size is <=256 bytes, so we should always have received all the data
                    // by this point
                    THROW(ERROR_INVALID_STATE);
                }
                readCborContent(cdata, dataLength);
                break;
            default:
                THROW(ERROR_INVALID_STATE);
        }

        if (ctx->dataLength == 0) {
            uiRegisterDataPayloadDisplay(flags);
        } else {
            sendSuccessNoIdle();
        }
    } else {
        THROW(ERROR_INVALID_STATE);
    }
}
