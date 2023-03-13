#include <os.h>

#include "responseCodes.h"
#include "sign.h"
#include "util.h"

static updateContractContext_t *ctx = &global.signUpdateContract;
static tx_state_t *tx_state = &global_tx_state;

#define P1_INITIAL          0x00
#define P1_RECEIVE_NAME 0x01
#define P1_PARAMETER     0x02

void handleSignUpdateContract(uint8_t *cdata, uint8_t p1, uint8_t dataLength, volatile unsigned int *flags, bool isInitialCall) {
    if (isInitialCall) {
        ctx->state = TX_UPDATE_CONTRACT_INITIAL;
    }

    if (p1 == P1_INITIAL && ctx->state == TX_UPDATE_CONTRACT_INITIAL) {
        cdata += parseKeyDerivationPath(cdata);
        cx_sha256_init(&tx_state->hash);
        cdata += hashAccountTransactionHeaderAndKind(cdata, UPDATE);

        // Build display value of the amount to transfer to the contract, and also add the bytes to the hash.
        uint64_t amount = U8BE(cdata, 0);
        amountToGtuDisplay(ctx->displayAmount, sizeof(ctx->displayAmount), amount);
        cx_hash((cx_hash_t *) &tx_state->hash, 0, cdata, 8, NULL, 0);
        cdata += 8;

        // Build display value of the index of the contract, and also add the bytes to the hash.
        uint64_t index = U8BE(cdata, 0);
        amountToGtuDisplay(ctx->displayIndex, sizeof(ctx->displayIndex), index);
        cx_hash((cx_hash_t *) &tx_state->hash, 0, cdata, 8, NULL, 0);
        cdata += 8;

        // Build display value of the subindex of the contract, and also add the bytes to the hash.
        uint64_t subindex = U8BE(cdata, 0);
        amountToGtuDisplay(ctx->displaySubindex, sizeof(ctx->displaySubindex), subindex);
        cx_hash((cx_hash_t *) &tx_state->hash, 0, cdata, 8, NULL, 0);
        cdata += 8;

        // Save the length of the receiveName and add the bytes to the hash
        ctx->nameLength = U2BE(cdata, 0);
        cx_hash((cx_hash_t *) &tx_state->hash, 0, cdata, 2, NULL, 0);
        cdata += 2;

        // Save the length of the parameters
        ctx->paramLength = U2BE(cdata, 0);
        memmove(ctx->rawParameterLength, cdata, 2);

        if (ctx->nameLength == 0) {
            THROW(ERROR_INVALID_PARAM);
        }

        // ctx->displayStart = true;
        ctx->state = TX_UPDATE_CONTRACT_RECEIVE_NAME;
        sendSuccessNoIdle();
    } else if (p1 == P1_RECEIVE_NAME && ctx->state == TX_UPDATE_CONTRACT_RECEIVE_NAME) {
        ctx->nameLength -= dataLength;
        cx_hash((cx_hash_t *) &tx_state->hash, 0, cdata, dataLength, NULL, 0);

        if (ctx->nameLength < 0) {
            THROW(ERROR_INVALID_STATE);
        } else if (ctx->nameLength == 0) {
            // Hash the parameterLength
            cx_hash((cx_hash_t *) &tx_state->hash, 0, ctx->rawParameterLength, 2, NULL, 0);
            ctx->state = TX_UPDATE_CONTRACT_PARAMETER;
        }

        // TODO Display start / receiveName
        sendSuccessNoIdle();
    } else if (p1 == P1_PARAMETER && ctx->state == TX_UPDATE_CONTRACT_PARAMETER) {
        ctx->paramLength -= dataLength;
        cx_hash((cx_hash_t *) &tx_state->hash, 0, cdata, dataLength, NULL, 0);

        if (ctx->paramLength < 0) {
            THROW(ERROR_INVALID_PARAM);
        } else if (ctx->paramLength == 0) {
            ctx->state = TX_UPDATE_CONTRACT_RECEIVE_NAME;

            ux_flow_init(0, ux_sign_flow_shared, NULL);
            *flags |= IO_ASYNCH_REPLY;
        } else {
            sendSuccessNoIdle();
        }
    } else {
        THROW(ERROR_INVALID_STATE);
    }
}
