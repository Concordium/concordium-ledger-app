#include <os.h>
#include <ux.h>
#include "util.h"
#include "accountSenderView.h"
#include "sign.h"
#include "responseCodes.h"

static signRegisterData_t *ctx = &global.withDataBlob.signRegisterData;
static cborContext_t *data_ctx = &global.withDataBlob.cborContext;
static tx_state_t *tx_state = &global_tx_state;

void handleData();

UX_STEP_VALID(ux_register_data_initial_flow_step, nn, sendSuccessNoIdle(), {"Continue", "with transaction"});
UX_STEP_VALID(
    ux_register_data_display_data_step,
    bnnn_paging,
    handleData(),
    {"Data", (char *) global.withDataBlob.cborContext.display});
UX_FLOW(
    ux_register_data_initial,
    &ux_sign_flow_shared_review,
    &ux_sign_flow_account_sender_view,
    &ux_register_data_initial_flow_step);

UX_FLOW(ux_register_data_payload, &ux_register_data_display_data_step);

void handleData() {
    if (ctx->dataLength > 0) {
        sendSuccessNoIdle();
    } else {
        ux_flow_init(0, ux_sign_flow_shared, NULL);
    }
}

#define P1_INITIAL 0x00
#define P1_DATA    0x01

void handleSignRegisterData(
    uint8_t *cdata,
    uint8_t p1,
    uint8_t dataLength,
    volatile unsigned int *flags,
    bool isInitialCall) {
    if (isInitialCall) {
        ctx->state = TX_REGISTER_DATA_INITIAL;
    }

    if (p1 == P1_INITIAL && ctx->state == TX_REGISTER_DATA_INITIAL) {
        cdata += parseKeyDerivationPath(cdata);
        cx_sha256_init(&tx_state->hash);
        cdata += hashAccountTransactionHeaderAndKind(cdata, REGISTER_DATA);

        // hash the data length
        ctx->dataLength = U2BE(cdata, 0);
        if (ctx->dataLength > MAX_DATA_SIZE) {
            THROW(ERROR_INVALID_PARAM);
        }
        data_ctx->cborLength = ctx->dataLength;
        updateHash((cx_hash_t *) &tx_state->hash, cdata, 2);

        ctx->state = TX_REGISTER_DATA_PAYLOAD_START;
        ux_flow_init(0, ux_register_data_initial, NULL);
        *flags |= IO_ASYNCH_REPLY;
    } else if (p1 == P1_DATA) {
        if (ctx->dataLength < dataLength) {
            THROW(ERROR_INVALID_TRANSACTION);
        }
        ctx->dataLength -= dataLength;
        updateHash((cx_hash_t *) &tx_state->hash, cdata, dataLength);

        switch (ctx->state) {
            case TX_REGISTER_DATA_PAYLOAD_START:
                ctx->state = TX_REGISTER_DATA_PAYLOAD;
                readCborInitial(cdata, dataLength);
                break;
            case TX_REGISTER_DATA_PAYLOAD:
                if (ctx->dataLength != 0) {
                    // The data size is <=256 bytes, so we should always have received all the data by this point
                    THROW(ERROR_INVALID_STATE);
                }
                readCborContent(cdata, dataLength);
                break;
            default:
                THROW(ERROR_INVALID_STATE);
        }

        if (ctx->dataLength == 0) {
            ux_flow_init(0, ux_register_data_payload, NULL);
            *flags |= IO_ASYNCH_REPLY;
        } else {
            sendSuccessNoIdle();
        }
    } else {
        THROW(ERROR_INVALID_STATE);
    }
}
