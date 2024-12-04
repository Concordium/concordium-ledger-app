#include <os.h>

#include "common/ui/display.h"
#include "responseCodes.h"
#include "sign.h"
#include "util.h"
#include "signConfigureDelegation.h"
#include "globals.h"

static signConfigureDelegationContext_t *ctx = &global.signConfigureDelegation;
static tx_state_t *tx_state = &global_tx_state;

void handleSignConfigureDelegation(uint8_t *cdata,
                                   uint8_t dataLength,
                                   volatile unsigned int *flags) {
    int keyDerivationPathLength = parseKeyDerivationPath(cdata);
    cdata += keyDerivationPathLength;

    cx_sha256_init(&tx_state->hash);
    int accountTransactionHeaderAndKindLength =
        hashAccountTransactionHeaderAndKind(cdata, CONFIGURE_DELEGATION);
    cdata += accountTransactionHeaderAndKindLength;

    // The initial 2 bytes tells us the fields we are receiving.
    updateHash((cx_hash_t *) &tx_state->hash, cdata, 2);
    uint16_t bitmap = U2BE(cdata, 0);
    cdata += 2;
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
        uint64_t capitalAmount = U8BE(cdata, 0);
        if (capitalAmount == 0) {
            ctx->stopDelegation = true;
        } else {
            ctx->stopDelegation = false;
            amountToGtuDisplay(ctx->displayCapital, sizeof(ctx->displayCapital), capitalAmount);
        }
        updateHash((cx_hash_t *) &tx_state->hash, cdata, 8);
        expectedDataLength += 8;
        cdata += 8;
    }

    if (ctx->hasRestakeEarnings) {
        uint8_t restake = cdata[0];
        updateHash((cx_hash_t *) &tx_state->hash, cdata, 1);
        expectedDataLength += 1;
        cdata += 1;
        if (restake == 0) {
            memmove(ctx->displayRestake, "No", 3);
        } else if (restake == 1) {
            memmove(ctx->displayRestake, "Yes", 4);
        } else {
            THROW(ERROR_INVALID_TRANSACTION);
        }
    }

    if (ctx->hasDelegationTarget) {
        uint8_t delegationType = cdata[0];
        updateHash((cx_hash_t *) &tx_state->hash, cdata, 1);
        expectedDataLength += 1;
        cdata += 1;

        if (delegationType == 0) {
            memmove(ctx->displayDelegationTarget, "Passive Delegation", 19);
        } else if (delegationType == 1) {
            uint64_t bakerId = U8BE(cdata, 0);
            expectedDataLength += 8;
            memmove(ctx->displayDelegationTarget, "Baker ID ", 9);
            bin2dec(ctx->displayDelegationTarget + 9, 21, bakerId);
            updateHash((cx_hash_t *) &tx_state->hash, cdata, 8);
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
