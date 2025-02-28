#include "globals.h"

static signConfigureDelegationContext_t *ctx = &global.signConfigureDelegation;
static tx_state_t *tx_state = &global_tx_state;

void handleSignConfigureDelegation(uint8_t *cdata,
                                   uint8_t dataLength,
                                   volatile unsigned int *flags) {
    int keyDerivationPathLength = parseKeyDerivationPath(cdata, dataLength);
    if (keyDerivationPathLength > dataLength) {
        THROW(ERROR_BUFFER_OVERFLOW);  // Ensure safe access
    }
    cdata += keyDerivationPathLength;

    if (cx_sha256_init(&tx_state->hash) != CX_SHA256) {
        THROW(ERROR_FAILED_CX_OPERATION);
    }
    int accountTransactionHeaderAndKindLength =
        hashAccountTransactionHeaderAndKind(cdata, dataLength, CONFIGURE_DELEGATION);
    if (accountTransactionHeaderAndKindLength > dataLength) {
        THROW(ERROR_BUFFER_OVERFLOW);  // Ensure safe access
    }
    cdata += accountTransactionHeaderAndKindLength;
    uint8_t remainingDataLength = dataLength - accountTransactionHeaderAndKindLength;

    // The initial 2 bytes tells us the fields we are receiving.
    if (remainingDataLength < 2) {
        THROW(ERROR_BUFFER_OVERFLOW);
    }
    updateHash((cx_hash_t *)&tx_state->hash, cdata, 2);
    uint16_t bitmap = U2BE(cdata, 0);
    cdata += 2;
    remainingDataLength -= 2;
    uint8_t expectedDataLength =
        keyDerivationPathLength + accountTransactionHeaderAndKindLength + 2;

    ctx->hasCapital = (bitmap >> 0) & 1;
    ctx->hasRestakeEarnings = (bitmap >> 1) & 1;
    ctx->hasDelegationTarget = (bitmap >> 2) & 1;

    // The transaction is invalid if neither of the optional fields are available.
    if (!ctx->hasCapital && !ctx->hasRestakeEarnings && !ctx->hasDelegationTarget) {
        THROW(ERROR_INVALID_TRANSACTION);
    }

    if (ctx->hasCapital) {
        if (remainingDataLength < 8) {
            THROW(ERROR_BUFFER_OVERFLOW);
        }
        uint64_t capitalAmount = U8BE(cdata, 0);
        if (capitalAmount == 0) {
            ctx->stopDelegation = true;
        } else {
            ctx->stopDelegation = false;
            amountToGtuDisplay(ctx->displayCapital, sizeof(ctx->displayCapital), capitalAmount);
        }
        updateHash((cx_hash_t *)&tx_state->hash, cdata, 8);
        expectedDataLength += 8;
        cdata += 8;
        remainingDataLength -= 8;
    }

    if (ctx->hasRestakeEarnings) {
        if (remainingDataLength < 1) {
            THROW(ERROR_BUFFER_OVERFLOW);
        }
        uint8_t restake = cdata[0];
        updateHash((cx_hash_t *)&tx_state->hash, cdata, 1);
        expectedDataLength += 1;
        cdata += 1;
        remainingDataLength -= 1;
        if (restake == 0) {
            memmove(ctx->displayRestake, "No", 3);
        } else if (restake == 1) {
            memmove(ctx->displayRestake, "Yes", 4);
        } else {
            THROW(ERROR_INVALID_TRANSACTION);
        }
    }

    if (ctx->hasDelegationTarget) {
        if (remainingDataLength < 1) {
            THROW(ERROR_BUFFER_OVERFLOW);
        }
        uint8_t delegationType = cdata[0];
        updateHash((cx_hash_t *)&tx_state->hash, cdata, 1);
        expectedDataLength += 1;
        cdata += 1;
        remainingDataLength -= 1;
        if (delegationType == 0) {
            memmove(ctx->displayDelegationTarget, "Passive Delegation", 19);
        } else if (delegationType == 1) {
            if (remainingDataLength < 8) {
                THROW(ERROR_BUFFER_OVERFLOW);
            }
            uint64_t bakerId = U8BE(cdata, 0);
            expectedDataLength += 8;
            memmove(ctx->displayDelegationTarget, "Baker ID ", 9);
            bin2dec(ctx->displayDelegationTarget + 9, 21, bakerId);
            updateHash((cx_hash_t *)&tx_state->hash, cdata, 8);
        } else {
            THROW(ERROR_INVALID_TRANSACTION);
        }
    }

    // There was a mismatch between the transaction and the reported data length.
    if (dataLength != expectedDataLength) {
        THROW(ERROR_INVALID_TRANSACTION);
    }

    startConfigureDelegationDisplay();
    *flags |= IO_ASYNCH_REPLY;
}
