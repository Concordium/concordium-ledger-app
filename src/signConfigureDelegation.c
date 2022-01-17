#include <os.h>

#include "accountSenderView.h"
#include "responseCodes.h"
#include "sign.h"
#include "util.h"

static signConfigureDelegationContext_t *ctx = &global.signConfigureDelegation;
static tx_state_t *tx_state = &global_tx_state;

// There will at most be 8 UI steps when all 3 optional fields are available.
const ux_flow_step_t *ux_sign_configure_delegation[8];

UX_STEP_NOCB(
    ux_sign_configure_delegation_capital_step,
    bnnn_paging,
    {.title = "Capital", .text = (char *) global.signConfigureDelegation.displayCapital});

UX_STEP_NOCB(
    ux_sign_configure_delegation_restake_step,
    bnnn_paging,
    {.title = "Restake earnings", .text = (char *) global.signConfigureDelegation.displayRestake});

UX_STEP_NOCB(
    ux_sign_configure_delegation_pool_step,
    bnnn_paging,
    {.title = "Delegation pool", .text = (char *) global.signConfigureDelegation.displayDelegationTarget});

void startDisplay() {
    uint8_t index = 0;

    ux_sign_configure_delegation[index++] = &ux_sign_flow_shared_review;
    ux_sign_configure_delegation[index++] = &ux_sign_flow_account_sender_view;

    if (ctx->hasCapital) {
        ux_sign_configure_delegation[index++] = &ux_sign_configure_delegation_capital_step;
    }
    
    if (ctx->hasRestakeEarnings) {
        ux_sign_configure_delegation[index++] = &ux_sign_configure_delegation_restake_step;
    }

    if (ctx->hasDelegationTarget) {
        ux_sign_configure_delegation[index++] = &ux_sign_configure_delegation_pool_step;
    }

    ux_sign_configure_delegation[index++] = &ux_sign_flow_shared_sign;
    ux_sign_configure_delegation[index++] = &ux_sign_flow_shared_decline;

    ux_sign_configure_delegation[index++] = FLOW_END_STEP;

    ux_flow_init(0, ux_sign_configure_delegation, NULL);
}

void handleSignConfigureDelegation(uint8_t *cdata, volatile unsigned int *flags) {
    cdata += parseKeyDerivationPath(cdata);
    cx_sha256_init(&tx_state->hash);
    cdata += hashAccountTransactionHeaderAndKind(cdata, CONFIGURE_DELEGATION);

    // The initial 2 bytes tells us the fields we are receiving.
    cx_hash((cx_hash_t *) &tx_state->hash, 0, cdata, 2, NULL, 0);
    uint16_t bitmap = U2BE(cdata, 0);
    cdata += 2;

    ctx->hasCapital = (bitmap >> 0) & 1;
    ctx->hasRestakeEarnings = (bitmap >> 1) & 1;
    ctx->hasDelegationTarget = (bitmap >> 2) & 1;

    // The transaction is invalid if neither of the optional fields are available.
    if (!ctx->hasCapital && !ctx->hasRestakeEarnings && !ctx->hasDelegationTarget) {
        THROW(ERROR_INVALID_TRANSACTION);
    }

    if (ctx->hasCapital) {
        uint64_t capitalAmount = U8BE(cdata, 0);
        amountToGtuDisplay(ctx->displayCapital, sizeof(ctx->displayCapital), capitalAmount);
        cx_hash((cx_hash_t *) &tx_state->hash, 0, cdata, 8, NULL, 0);
        cdata += 8;
    }

    if (ctx->hasRestakeEarnings) {
        uint8_t restake = cdata[0];
        cx_hash((cx_hash_t *) &tx_state->hash, 0, cdata, 1, NULL, 0);
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
        cx_hash((cx_hash_t *) &tx_state->hash, 0, cdata, 1, NULL, 0);
        cdata += 1;

        if (delegationType == 0) {
            memmove(ctx->displayDelegationTarget, "L-pool", 7);
        } else if (delegationType == 1) {
            uint64_t bakerId = U8BE(cdata, 0);
            memmove(ctx->displayDelegationTarget, "Baker ID ", 9);
            bin2dec(ctx->displayDelegationTarget + 9, 21, bakerId);
            cx_hash((cx_hash_t *) &tx_state->hash, 0, cdata, 8, NULL, 0);
        } else {
            THROW(ERROR_INVALID_TRANSACTION);
        }
    }

    startDisplay();
    *flags |= IO_ASYNCH_REPLY;
}
