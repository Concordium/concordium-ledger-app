#include <os.h>

#include "base58check.h"
#include "responseCodes.h"
#include "sign.h"
#include "util.h"

static signCreatePltContext_t *ctx = &global.signCreatePltContext;
static tx_state_t *tx_state = &global_tx_state;

UX_STEP_NOCB(
    ux_sign_create_plt_token_symbol,
    bnnn_paging,
    {"Token Symbol", (char *) global.signCreatePltContext.tokenSymbol});

UX_STEP_NOCB(
    ux_sign_create_plt_token_module,
    bnnn_paging,
    {"Token Module", (char *) global.signCreatePltContext.tokenModule});



UX_STEP_NOCB(
    ux_sign_create_plt_decimals,
    bn,
    {"Decimals", (char *) global.signCreatePltContext.decimals});

UX_STEP_NOCB(
    ux_sign_create_plt_init_params,
    bnnn_paging,
    {"Init Params", (char *) global.signCreatePltContext.initParamsHex});

UX_FLOW(ux_sign_create_plt_start, 
    &ux_sign_flow_shared_review, 
    &ux_sign_create_plt_token_symbol,
    &ux_sign_create_plt_token_module,
    &ux_sign_create_plt_decimals,
    &ux_sign_create_plt_init_params,
    &ux_sign_flow_shared_sign,
    &ux_sign_flow_shared_decline);

#define P1_INITIAL                      0x00
#define P1_PAYLOAD                      0x01
#define P1_INIT_PARAMS                  0x02

void handleSignCreatePlt(
    uint8_t *cdata,
    uint8_t p1,
    uint8_t dataLength,
    volatile unsigned int *flags,
    bool isInitialCall) {
    
    if (isInitialCall) {
        ctx->state = TX_CREATE_PLT_INITIAL;
    }

    if (p1 == P1_INITIAL && ctx->state == TX_CREATE_PLT_INITIAL) {
        cdata += parseKeyDerivationPath(cdata);

        cx_sha256_init(&tx_state->hash);
        cdata += hashUpdateHeaderAndType(cdata, UPDATE_TYPE_CREATE_PLT);

        // Read the total payload length (excluding init params)
        ctx->payloadLength = U8BE(cdata, 0);
        cx_hash((cx_hash_t *) &tx_state->hash, 0, cdata, 8, NULL, 0);
        cdata += 8;

        ctx->state = TX_CREATE_PLT_PAYLOAD;
        sendSuccessNoIdle();

    } else if (p1 == P1_PAYLOAD && ctx->state == TX_CREATE_PLT_PAYLOAD) {
        // Parse token symbol length
        ctx->tokenSymbolLength = U4BE(cdata, 0);
        cx_hash((cx_hash_t *) &tx_state->hash, 0, cdata, 4, NULL, 0);
        cdata += 4;

        if (ctx->tokenSymbolLength > 128) {
            THROW(ERROR_INVALID_TRANSACTION);
        }

        // Parse token symbol
        if (ctx->tokenSymbolLength > 0) {
            memmove(ctx->tokenSymbol, cdata, ctx->tokenSymbolLength);
            ctx->tokenSymbol[ctx->tokenSymbolLength] = '\0';
            cx_hash((cx_hash_t *) &tx_state->hash, 0, cdata, ctx->tokenSymbolLength, NULL, 0);
            cdata += ctx->tokenSymbolLength;
        }

        // Parse token module (32 bytes)
        toPaginatedHex(cdata, 32, ctx->tokenModule, sizeof(ctx->tokenModule));
        cx_hash((cx_hash_t *) &tx_state->hash, 0, cdata, 32, NULL, 0);
        cdata += 32;



        // Parse decimals (1 byte)
        uint8_t decimalsValue = cdata[0];
        cx_hash((cx_hash_t *) &tx_state->hash, 0, cdata, 1, NULL, 0);
        cdata += 1;
        bin2dec(ctx->decimals, sizeof(ctx->decimals), decimalsValue);

        // Parse initialization parameters length (4 bytes)
        ctx->initializationParamsLength = U4BE(cdata, 0);
        cx_hash((cx_hash_t *) &tx_state->hash, 0, cdata, 4, NULL, 0);
        cdata += 4;
        ctx->remainingInitializationParamsBytes = ctx->initializationParamsLength;

        // Initialization parameters are required for PLT creation
        if (ctx->initializationParamsLength == 0) {
            THROW(ERROR_INVALID_TRANSACTION);
        }

        // Initialize init params storage
        memset(ctx->initParams, 0, sizeof(ctx->initParams));
        memset(ctx->initParamsHex, 0, sizeof(ctx->initParamsHex));

        ctx->state = TX_CREATE_PLT_INIT_PARAMS;
        sendSuccessNoIdle();

    } else if (p1 == P1_INIT_PARAMS && ctx->state == TX_CREATE_PLT_INIT_PARAMS) {
        if (ctx->remainingInitializationParamsBytes < dataLength) {
            THROW(ERROR_INVALID_STATE);
        }

        // Store init params data (up to 256 bytes for display)
        // Note: If init params exceed 256 bytes, only the first 256 bytes will be displayed
        uint32_t currentOffset = ctx->initializationParamsLength - ctx->remainingInitializationParamsBytes;
        uint32_t bytesToStore = dataLength;
        if (currentOffset + bytesToStore > sizeof(ctx->initParams)) {
            bytesToStore = sizeof(ctx->initParams) - currentOffset;
        }
        if (bytesToStore > 0 && currentOffset < sizeof(ctx->initParams)) {
            memmove(ctx->initParams + currentOffset, cdata, bytesToStore);
        }

        // Hash the initialization parameters (all data is hashed, even if not all is displayed)
        cx_hash((cx_hash_t *) &tx_state->hash, 0, cdata, dataLength, NULL, 0);
        
        ctx->remainingInitializationParamsBytes -= dataLength;

        if (ctx->remainingInitializationParamsBytes == 0) {
            // All initialization parameters received, convert to hex for display
            uint32_t displayLength = ctx->initializationParamsLength;
            if (displayLength > sizeof(ctx->initParams)) {
                displayLength = sizeof(ctx->initParams);
            }
            
            // Convert stored params to hex for display
            toPaginatedHex(ctx->initParams, displayLength, ctx->initParamsHex, sizeof(ctx->initParamsHex));
            
            // If init params were truncated, append indicator
            if (ctx->initializationParamsLength > sizeof(ctx->initParams)) {
                size_t currentLen = strlen(ctx->initParamsHex);
                if (currentLen < sizeof(ctx->initParamsHex) - 15) {
                    strcat(ctx->initParamsHex, "...(truncated)");
                }
            }
            
            // Show UI for user review
            ux_flow_init(0, ux_sign_create_plt_start, NULL);
            *flags |= IO_ASYNCH_REPLY;
        } else {
            // More initialization parameters to receive
            sendSuccessNoIdle();
        }

    } else {
        THROW(ERROR_INVALID_STATE);
    }
}