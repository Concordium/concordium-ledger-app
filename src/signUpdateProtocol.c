#include <os.h>
#include <os_io_seproxyhal.h>
#include "cx.h"
#include <stdint.h>
#include "menu.h"
#include "util.h"
#include <string.h>
#include "sign.h"

static signUpdateProtocolContext_t *ctx = &global.signUpdateProtocolContext;
static tx_state_t *tx_state = &global_tx_state;

void handleText();

UX_STEP_NOCB(
    ux_sign_protocol_update_0_step,
    nn,
    {
      "Review",
      "transaction"
    });
UX_STEP_CB(
    ux_sign_protocol_update_1_step,
    bn_paging,
    handleText(),
    {
        "Message",
        (char *) global.signUpdateProtocolContext.buffer
    });
UX_FLOW(ux_sign_protocol_update,
    &ux_sign_protocol_update_0_step,
    &ux_sign_protocol_update_1_step
);

UX_STEP_CB(
    ux_sign_protocol_update_url_0_step,
    bn_paging,
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
    bn_paging,
    sendSuccessNoIdle(),
    {
        "Spec. hash",
        (char *) global.signUpdateProtocolContext.specificationHash
    });
UX_FLOW(ux_sign_protocol_update_specification_hash,
    &ux_sign_protocol_update_specification_hash_0_step
);

void handleText() {
    if (ctx->textLength == 0) {
        ctx->textState += 1;
    }
    sendSuccessNoIdle();
}

#define P1_INITIAL              0x00
#define P1_TEXT_LENGTH          0x01        // Used for both the message text and the specificaiton URL.
#define P1_TEXT                 0x02        // Used for both the message text and the specification URL.
#define P1_SPECIFICATION_HASH   0x03  
#define P1_AUXILIARY_DATA       0x04

void handleSignUpdateProtocol(uint8_t *dataBuffer, uint8_t p1, uint8_t dataLength, volatile unsigned int *flags) {
    if (p1 == P1_INITIAL) {
        int bytesRead = parseKeyDerivationPath(dataBuffer);
        dataBuffer += bytesRead;

        cx_sha256_init(&tx_state->hash);

        // Hash update header and update kind.
        cx_hash((cx_hash_t *) &tx_state->hash, 0, dataBuffer, UPDATE_HEADER_LENGTH + 1, NULL, 0);
        dataBuffer += UPDATE_HEADER_LENGTH + 1;

        // Read payload length.
        ctx->payloadLength = U8BE(dataBuffer, 0);
        cx_hash((cx_hash_t *) &tx_state->hash, 0, dataBuffer, 8, NULL, 0);
        dataBuffer += 8;

        ctx->textState = MESSAGE;

        sendSuccessNoIdle();
    } else if (p1 == P1_TEXT_LENGTH) {
        
        // Read message text length
        ctx->textLength = U8BE(dataBuffer, 0);
        cx_hash((cx_hash_t *) &tx_state->hash, 0, dataBuffer, 8, NULL, 0);
        dataBuffer += 8;

        // Update payload length to ensure we end up with the length of the auxiliary data.
        ctx->payloadLength -= 8;

        sendSuccessNoIdle();
    } else if (p1 == P1_TEXT) {
        if (ctx->textLength <= 0) {
            THROW(SW_INVALID_STATE);
        }

        os_memmove(ctx->buffer, dataBuffer, dataLength);
        cx_hash((cx_hash_t *) &tx_state->hash, 0, dataBuffer, dataLength, NULL, 0);
        ctx->textLength -= dataLength;
        ctx->payloadLength -= dataLength;

        if (dataLength < 255) {
            os_memmove(ctx->buffer + dataLength, "\0", 1);
        }

        switch (ctx->textState) {
            case MESSAGE: 
                ux_flow_init(0, ux_sign_protocol_update, NULL);
                break;
            case SPECIFICATION_URL: 
                ux_flow_init(0, ux_sign_protocol_update_url, NULL);
                break;
            default: 
                THROW(SW_INVALID_STATE);
                break;
        }

        *flags |= IO_ASYNCH_REPLY;
    } else if (p1 == P1_SPECIFICATION_HASH) {
        cx_hash((cx_hash_t *) &tx_state->hash, 0, dataBuffer, 32, NULL, 0);
        toHex(dataBuffer, 32, ctx->specificationHash);
        ctx->payloadLength -= 32;

        ux_flow_init(0, ux_sign_protocol_update_specification_hash, NULL);
        *flags |= IO_ASYNCH_REPLY;
    } else if (p1 == P1_AUXILIARY_DATA) {
        cx_hash((cx_hash_t *) &tx_state->hash, 0, dataBuffer, dataLength, NULL, 0);
        ctx->payloadLength -= dataLength;

        if (ctx->payloadLength == 0) {
            // We have received all the auxiliary data bytes, continue to signing flow.
            ux_flow_init(0, ux_sign_flow_shared, NULL);
            *flags |= IO_ASYNCH_REPLY;
        } else if (ctx->payloadLength > 0) {
            // Ask for more bytes as we have not received all of them yet.
            sendSuccessNoIdle();
        } else {
            THROW(SW_INVALID_STATE);
        }
    } else {
        THROW(SW_INVALID_STATE);
    }
}
