#include <os.h>
#include "util.h"
#include "sign.h"
#include "responseCodes.h"

static signUpdateProtocolContext_t *ctx = &global.signUpdateProtocolContext;
static tx_state_t *tx_state = &global_tx_state;

void handleText();
void switchToLoading();

UX_STEP_CB(
    ux_sign_protocol_update_1_step,
    bnnn_paging,
    handleText(),
    {
        "Message",
        (char *) global.signUpdateProtocolContext.buffer
    });
UX_FLOW(ux_sign_protocol_update,
    &ux_sign_flow_shared_review,
    &ux_sign_protocol_update_1_step
);

UX_STEP_CB(
    ux_sign_protocol_update_url_0_step,
    bnnn_paging,
    handleText(),
    {
        "Spec. URL",
        (char *) global.signUpdateProtocolContext.buffer
    });
UX_FLOW(ux_sign_protocol_update_url,
    &ux_sign_protocol_update_url_0_step
);

UX_STEP_CB(
    ux_sign_protocol_update_specification_hash_0_step,
    bnnn_paging,
    switchToLoading(),
    {
        "Spec. hash",
        (char *) global.signUpdateProtocolContext.specificationHash
    });
UX_FLOW(ux_sign_protocol_update_specification_hash,
    &ux_sign_protocol_update_specification_hash_0_step
);

UX_STEP_NOCB(
    ux_sign_protocol_update_loading_step,
    nn,
    {
        "Loading data,",
        "please wait"
    });
UX_FLOW(ux_sign_protocol_update_loading,
    &ux_sign_protocol_update_loading_step
);

void switchToLoading(void) {
    ux_flow_init(0, ux_sign_protocol_update_loading, NULL);
    sendSuccessNoIdle();
}

void handleText(void) {
    if (ctx->textLength == 0) {
        ctx->textState += 1;
    }
    sendSuccessNoIdle();
}

#define P1_INITIAL              0x00
#define P1_TEXT_LENGTH          0x01        // Used for both the message text and the specification URL.
#define P1_TEXT                 0x02        // Used for both the message text and the specification URL.
#define P1_SPECIFICATION_HASH   0x03
#define P1_AUXILIARY_DATA       0x04

void handleSignUpdateProtocol(uint8_t *cdata, uint8_t p1, uint8_t dataLength, volatile unsigned int *flags, bool isInitialCall) {
    if (isInitialCall) {
        ctx->state = TX_UPDATE_PROTOCOL_INITIAL;
    }

    if (p1 == P1_INITIAL && ctx->state == TX_UPDATE_PROTOCOL_INITIAL) {
        int bytesRead = parseKeyDerivationPath(cdata);
        cdata += bytesRead;

        cx_sha256_init(&tx_state->hash);
        cdata += hashUpdateHeaderAndType(cdata, UPDATE_TYPE_PROTOCOL);

        // Read payload length.
        ctx->payloadLength = U8BE(cdata, 0);
        cx_hash((cx_hash_t *) &tx_state->hash, 0, cdata, 8, NULL, 0);

        ctx->textState = MESSAGE;
        ctx->state = TX_UPDATE_PROTOCOL_TEXT_LENGTH;

        sendSuccessNoIdle();
    } else if (p1 == P1_TEXT_LENGTH && ctx->state == TX_UPDATE_PROTOCOL_TEXT_LENGTH) {
        // Read message text length
        ctx->textLength = U8BE(cdata, 0);
        cx_hash((cx_hash_t *) &tx_state->hash, 0, cdata, 8, NULL, 0);

        // Update payload length to ensure we end up with the length of the auxiliary data.
        ctx->payloadLength -= 8;

        ctx->state = TX_UPDATE_PROTOCOL_TEXT;
        sendSuccessNoIdle();
    } else if (p1 == P1_TEXT && ctx->state == TX_UPDATE_PROTOCOL_TEXT) {
        if (ctx->textLength <= 0) {
            THROW(ERROR_INVALID_STATE);
        }

        memmove(ctx->buffer, cdata, dataLength);
        cx_hash((cx_hash_t *) &tx_state->hash, 0, cdata, dataLength, NULL, 0);
        ctx->textLength -= dataLength;
        ctx->payloadLength -= dataLength;

        if (dataLength < 255) {
            memmove(ctx->buffer + dataLength, "\0", 1);
        }

        switch (ctx->textState) {
            case MESSAGE:
                ctx->state = TX_UPDATE_PROTOCOL_TEXT_LENGTH;
                ux_flow_init(0, ux_sign_protocol_update, NULL);
                break;
            case SPECIFICATION_URL:
                ctx->state = TX_UPDATE_PROTOCOL_SPECIFICATION_HASH;
                ux_flow_init(0, ux_sign_protocol_update_url, NULL);
                break;
            default:
                THROW(ERROR_INVALID_STATE);
                break;
        }

        *flags |= IO_ASYNCH_REPLY;
    } else if (p1 == P1_SPECIFICATION_HASH && ctx->state == TX_UPDATE_PROTOCOL_SPECIFICATION_HASH) {
        cx_hash((cx_hash_t *) &tx_state->hash, 0, cdata, 32, NULL, 0);
        toPaginatedHex(cdata, 32, ctx->specificationHash);
        ctx->payloadLength -= 32;

        ctx->state = TX_UPDATE_PROTOCOL_AUXILIARY_DATA;
        ux_flow_init(0, ux_sign_protocol_update_specification_hash, NULL);
        *flags |= IO_ASYNCH_REPLY;
    } else if (p1 == P1_AUXILIARY_DATA && ctx->state == TX_UPDATE_PROTOCOL_AUXILIARY_DATA) {
        cx_hash((cx_hash_t *) &tx_state->hash, 0, cdata, dataLength, NULL, 0);
        ctx->payloadLength -= dataLength;

        if (ctx->payloadLength == 0) {
            // We have received all the auxiliary data bytes, continue to signing flow.
            ux_flow_init(0, ux_sign_flow_shared, NULL);
            *flags |= IO_ASYNCH_REPLY;
        } else if (ctx->payloadLength > 0) {
            // Ask for more bytes as we have not received all of them yet.
            sendSuccessNoIdle();
        } else {
            THROW(ERROR_INVALID_STATE);
        }
    } else {
        THROW(ERROR_INVALID_STATE);
    }
}
