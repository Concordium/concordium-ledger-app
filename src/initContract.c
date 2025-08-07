#include "globals.h"

static initContract_t *ctx_init_contract = &global.initContract;
static tx_state_t *tx_state = &global_tx_state;

#define P1_INITIAL 0x00
#define P1_NAME    0x01
#define P1_PARAMS  0x02

void handleInitContract(uint8_t *cdata, uint8_t p1, uint8_t lc) {
    if (p1 == P1_INITIAL) {
        if (cx_sha256_init(&tx_state->hash) != CX_SHA256) {
            THROW(ERROR_FAILED_CX_OPERATION);
        }

        size_t offset = parseKeyDerivationPath(cdata, lc);
        if (offset > lc) {
            THROW(ERROR_BUFFER_OVERFLOW);  // Ensure safe access
        }
        cdata += offset;
        uint8_t remainingDataLength = lc - offset;

        offset = hashAccountTransactionHeaderAndKind(cdata, remainingDataLength, INIT_CONTRACT);
        if (offset > lc) {
            THROW(ERROR_BUFFER_OVERFLOW);  // Ensure safe access
        }
        cdata += offset;
        remainingDataLength -= offset;
        if (remainingDataLength < 8) {
            THROW(ERROR_BUFFER_OVERFLOW);
        }
        // hash the amount
        updateHash((cx_hash_t *)&tx_state->hash, cdata, 8);
        // extract the amount
        ctx_init_contract->amount = U8BE(cdata, 0);
        // Format the amount
        amountToGtuDisplay((uint8_t *)ctx_init_contract->amountDisplay,
                           sizeof(ctx_init_contract->amountDisplay),
                           ctx_init_contract->amount);
        cdata += 8;
        remainingDataLength -= 8;
        if (remainingDataLength < 32) {
            THROW(ERROR_BUFFER_OVERFLOW);
        }
        // hash the module ref
        updateHash((cx_hash_t *)&tx_state->hash, cdata, 32);
        // extract the module ref
        memmove(ctx_init_contract->moduleRef, cdata, 32);
        // Format the module ref
        if (format_hex(ctx_init_contract->moduleRef, 32, ctx_init_contract->moduleRefDisplay, 65) ==
            -1) {
            THROW(ERROR_INVALID_MODULE_REF);
        }
        ctx_init_contract->state = INIT_CONTRACT_NAME_FIRST;
        sendSuccessNoIdle();
    }

    else if (p1 == P1_NAME) {
        uint8_t lengthSize = 2;
        if (ctx_init_contract->state == INIT_CONTRACT_NAME_FIRST) {
            // extract the name length
            if (lc < 2) {
                THROW(ERROR_BUFFER_OVERFLOW);
            }
            ctx_init_contract->nameLength = U2BE(cdata, 0);
            // calculate the remaining name length
            ctx_init_contract->remainingNameLength = ctx_init_contract->nameLength + lengthSize;
            // set the state to the next state
            ctx_init_contract->state = INIT_CONTRACT_NAME_NEXT;
        } else if (ctx_init_contract->remainingNameLength < lc) {
            THROW(ERROR_INVALID_NAME_LENGTH);
        }
        // hash the whole chunk
        updateHash((cx_hash_t *)&tx_state->hash, cdata, lc);
        // subtract the length of the chunk from the remaining name length
        ctx_init_contract->remainingNameLength -= lc;
        if (ctx_init_contract->remainingNameLength > 0) {
            sendSuccessNoIdle();
        } else if (ctx_init_contract->remainingNameLength == 0) {
            ctx_init_contract->state = INIT_CONTRACT_PARAMS_FIRST;
            sendSuccessNoIdle();
        }

    } else if (p1 == P1_PARAMS) {
        uint8_t lengthSize = 2;
        if (ctx_init_contract->state == INIT_CONTRACT_PARAMS_FIRST) {
            // extract the params length
            if (lc < 2) {
                THROW(ERROR_BUFFER_OVERFLOW);
            }
            ctx_init_contract->paramsLength = U2BE(cdata, 0);
            // calculate the remaining params length
            ctx_init_contract->remainingParamsLength = ctx_init_contract->paramsLength + lengthSize;
            // set the state to the next state
            ctx_init_contract->state = INIT_CONTRACT_PARAMS_NEXT;
        } else if (ctx_init_contract->remainingParamsLength < lc) {
            THROW(ERROR_INVALID_PARAMS_LENGTH);
        }
        // hash the whole chunk
        updateHash((cx_hash_t *)&tx_state->hash, cdata, lc);
        // subtract the length of the chunk from the remaining params length
        ctx_init_contract->remainingParamsLength -= lc;
        if (ctx_init_contract->remainingParamsLength > 0) {
            sendSuccessNoIdle();
        } else if (ctx_init_contract->remainingParamsLength == 0) {
            uiInitContractDisplay();
        }

    } else {
        THROW(ERROR_INVALID_STATE);
    }
}