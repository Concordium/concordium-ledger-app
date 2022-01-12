#include <os.h>

#include "accountSenderView.h"
#include "responseCodes.h"
#include "sign.h"
#include "util.h"

static signAddBakerContext_t *ctx = &global.signAddBaker;
static tx_state_t *tx_state = &global_tx_state;

UX_STEP_NOCB(
    ux_sign_add_baker_1_step,
    bnnn_paging,
    {.title = "Amount to stake", .text = (char *) global.signAddBaker.amount});
UX_STEP_NOCB(
    ux_sign_add_baker_2_step,
    bnnn_paging,
    {.title = "Restake earnings", .text = (char *) global.signAddBaker.restake});
UX_FLOW(
    ux_sign_add_baker,
    &ux_sign_flow_shared_review,
    &ux_sign_flow_account_sender_view,
    &ux_sign_add_baker_1_step,
    &ux_sign_add_baker_2_step,
    &ux_sign_flow_shared_sign,
    &ux_sign_flow_shared_decline);

UX_STEP_NOCB(ux_sign_update_baker_keys_0_step, nn, {"Update baker", "keys"});
UX_FLOW(
    ux_sign_update_baker_keys,
    &ux_sign_flow_shared_review,
    &ux_sign_flow_account_sender_view,
    &ux_sign_update_baker_keys_0_step,
    &ux_sign_flow_shared_sign,
    &ux_sign_flow_shared_decline);

#define P1_INITIAL               0x00  // Key path, transaction header and kind.
#define P1_VERIFY_KEYS           0x01  // The three verification keys.
#define P1_PROOFS_AMOUNT_RESTAKE 0x02  // Proofs for the verification keys, stake amount and whether or not to restake.

#define P2_ADD_BAKER         0x00  // Sign an add baker transaction.
#define P2_UPDATE_BAKER_KEYS 0x01  // Sign an update baker keys transaction.

void handleSignAddBakerOrUpdateBakerKeys(
    uint8_t *cdata,
    uint8_t p1,
    uint8_t p2,
    volatile unsigned int *flags,
    bool isInitialCall) {
    if (p2 != P2_ADD_BAKER && p2 != P2_UPDATE_BAKER_KEYS) {
        THROW(ERROR_INVALID_PARAM);
    }

    if (isInitialCall) {
        ctx->state = ADD_BAKER_INITIAL;
    }

    if (p1 == P1_INITIAL && ctx->state == ADD_BAKER_INITIAL) {
        cdata += parseKeyDerivationPath(cdata);

        cx_sha256_init(&tx_state->hash);
        if (p2 == P2_ADD_BAKER) {
            hashAccountTransactionHeaderAndKind(cdata, ADD_BAKER);
        } else if (p2 == P2_UPDATE_BAKER_KEYS) {
            hashAccountTransactionHeaderAndKind(cdata, UPDATE_BAKER_KEYS);
        } else {
            THROW(ERROR_INVALID_TRANSACTION);
        }

        ctx->state = ADD_BAKER_VERIFY_KEYS;
        sendSuccessNoIdle();
    } else if (p1 == P1_VERIFY_KEYS && ctx->state == ADD_BAKER_VERIFY_KEYS) {
        // We do not display the verification keys to the user, as they are difficult
        // for the user to verify. If need be, we can start showing them by parsing
        // the values into hex strings here.

        // Election verify key
        cx_hash((cx_hash_t *) &tx_state->hash, 0, cdata, 32, NULL, 0);
        cdata += 32;

        // Baker sign verify key
        cx_hash((cx_hash_t *) &tx_state->hash, 0, cdata, 32, NULL, 0);
        cdata += 32;

        // Baker aggregation verify key
        cx_hash((cx_hash_t *) &tx_state->hash, 0, cdata, 96, NULL, 0);

        ctx->state = ADD_BAKER_PROOFS_AMOUNT_RESTAKE;
        sendSuccessNoIdle();
    } else if (p1 == P1_PROOFS_AMOUNT_RESTAKE && ctx->state == ADD_BAKER_PROOFS_AMOUNT_RESTAKE) {
        // Next comes 3x64 bytes of proofs corresponding to each of the
        // verification keys above. Proofs are not sensible to display,
        // so we add it directly to the hash.
        cx_hash((cx_hash_t *) &tx_state->hash, 0, cdata, 192, NULL, 0);
        cdata += 192;

        if (p2 == P2_UPDATE_BAKER_KEYS) {
            ux_flow_init(0, ux_sign_update_baker_keys, NULL);
            *flags |= IO_ASYNCH_REPLY;
        } else if (p2 == P2_ADD_BAKER) {
            // Parse the amount to stake, so it can be displayed for verification.
            uint64_t amount = U8BE(cdata, 0);
            amountToGtuDisplay(ctx->amount, sizeof(ctx->amount), amount);
            cx_hash((cx_hash_t *) &tx_state->hash, 0, cdata, 8, NULL, 0);
            cdata += 8;

            // Parse the bool (as 1 byte) of whether to restake earnings.
            uint8_t restakeEarnings = cdata[0];
            cx_hash((cx_hash_t *) &tx_state->hash, 0, cdata, 1, NULL, 0);
            if (restakeEarnings == 0) {
                memmove(ctx->restake, "No", 3);
            } else if (restakeEarnings == 1) {
                memmove(ctx->restake, "Yes", 4);
            } else {
                THROW(ERROR_INVALID_TRANSACTION);
            }

            ux_flow_init(0, ux_sign_add_baker, NULL);
            *flags |= IO_ASYNCH_REPLY;
        } else {
            THROW(ERROR_INVALID_PARAM);
        }
    } else {
        THROW(ERROR_INVALID_STATE);
    }
}
