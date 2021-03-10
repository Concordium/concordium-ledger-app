#include <os.h>
#include <os_io_seproxyhal.h>
#include "cx.h"
#include <stdint.h>
#include "menu.h"
#include "util.h"
#include <string.h>
#include "base58check.h"
#include <stdio.h>
#include "sign.h"

static signAddBakerContext_t *ctx = &global.signAddBaker;
static tx_state_t *tx_state = &global_tx_state;

UX_STEP_NOCB(
    ux_sign_add_baker_0_step,
    nn,
    {
      "Review",
      "transaction"
    });
UX_STEP_CB(
    ux_sign_add_baker_1_step,
    bn_paging,
    sendSuccessNoIdle(),
    {
      .title = "Amount to stake",
      .text = (char *) global.signAddBaker.amount
    });
UX_STEP_CB(
    ux_sign_add_baker_2_step,
    bn_paging,
    sendSuccessNoIdle(),
    {
        .title = "Restake earnings",
        .text = (char *) global.signAddBaker.restake
    });
UX_STEP_CB(
    ux_sign_add_baker_3_step,
    pnn,
    buildAndSignTransactionHash(),
    {
      &C_icon_validate_14,
      "Sign",
      "transaction"
    });
UX_STEP_CB(
    ux_sign_add_baker_4_step,
    pnn,
    declineToSignTransaction(),
    {
      &C_icon_crossmark,
      "Decline to",
      "sign transaction"
    });
UX_FLOW(ux_sign_add_baker,
    &ux_sign_add_baker_0_step,
    &ux_sign_add_baker_1_step,
    &ux_sign_add_baker_2_step,
    &ux_sign_add_baker_3_step,
    &ux_sign_add_baker_4_step
);

UX_STEP_NOCB(
    ux_sign_update_baker_keys_0_step,
    nn,
    {
      "Update baker",
      "keys"
    });
UX_FLOW(ux_sign_update_baker_keys,
    &ux_sign_add_baker_0_step,
    &ux_sign_update_baker_keys_0_step,
    &ux_sign_add_baker_3_step,
    &ux_sign_add_baker_4_step
);

#define P1_INITIAL                  0x00    // Key path, transaction header and kind.
#define P1_VERIFY_KEYS              0x01    // The three verification keys.
#define P1_PROOFS_AMOUNT_RESTAKE    0x02    // Proofs for the verification keys, stake amount and whether or not to restake.

#define P2_ADD_BAKER                0x00    // Sign an add baker transaction.
#define P2_UPDATE_BAKER_KEYS        0x01    // Sign an update baker keys transaction.

void handleSignAddBakerOrUpdateBakerKeys(uint8_t *dataBuffer, uint8_t p1, uint8_t p2, volatile unsigned int *flags) {
    if (p1 != P1_INITIAL && !tx_state->initialized) {
        THROW(SW_INVALID_STATE);
    }

    if (p2 != P2_ADD_BAKER && p2 != P2_UPDATE_BAKER_KEYS) {
        THROW(SW_INVALID_PARAM);
    }

    if (p1 == P1_INITIAL) {
        tx_state->initialized = true;

        int bytesRead = parseKeyDerivationPath(dataBuffer);
        dataBuffer += bytesRead;

        cx_sha256_init(&tx_state->hash);
        cx_hash((cx_hash_t *) &tx_state->hash, 0, dataBuffer, ACCOUNT_TRANSACTION_HEADER_LENGTH + 1, NULL, 0);
        uint8_t transactionKind = dataBuffer[ACCOUNT_TRANSACTION_HEADER_LENGTH];

        if ((p2 == P2_ADD_BAKER && transactionKind != ADD_BAKER) || (p2 == P2_UPDATE_BAKER_KEYS && transactionKind != UPDATE_BAKER_KEYS)) {
            THROW(SW_INVALID_TRANSACTION);
        }
        dataBuffer += ACCOUNT_TRANSACTION_HEADER_LENGTH + 1;

        ctx->state = ADD_BAKER_VERIFY_KEYS;
        sendSuccessNoIdle();
    } else if (p1 == P1_VERIFY_KEYS && ctx->state == ADD_BAKER_VERIFY_KEYS) {
        // We do not display the verification keys to the user, as they are difficult
        // for the user to verify. If need be, we can start showing them by parsing
        // the values into hex strings here.

        // Election verify key
        cx_hash((cx_hash_t *) &tx_state->hash, 0, dataBuffer, 32, NULL, 0);
        dataBuffer += 32;
        
        // Baker sign verify key
        cx_hash((cx_hash_t *) &tx_state->hash, 0, dataBuffer, 32, NULL, 0);
        dataBuffer += 32;

        // Baker aggregation verify key
        cx_hash((cx_hash_t *) &tx_state->hash, 0, dataBuffer, 96, NULL, 0);
        dataBuffer += 96;

        ctx->state = ADD_BAKER_PROOFS_AMOUNT_RESTAKE;
        sendSuccessNoIdle();
    } else if (p1 == P1_PROOFS_AMOUNT_RESTAKE && ctx->state == ADD_BAKER_PROOFS_AMOUNT_RESTAKE) {
        // Next comes 3x64 bytes of proofs corresponding to each of the 
        // verification keys above. Proofs are not sensible to display,
        // so we add it directly to the hash.
        cx_hash((cx_hash_t *) &tx_state->hash, 0, dataBuffer, 192, NULL, 0);
        dataBuffer += 192;

        if (p2 == P2_UPDATE_BAKER_KEYS) {
            ux_flow_init(0, ux_sign_update_baker_keys, NULL);
            *flags |= IO_ASYNCH_REPLY;
        } else if (p2 == P2_ADD_BAKER) {
            // Parse the amount to stake, so it can be displayed for verification.
            uint64_t amount = U8BE(dataBuffer, 0);
            bin2dec(ctx->amount, amount);
            cx_hash((cx_hash_t *) &tx_state->hash, 0, dataBuffer, 8, NULL, 0);
            dataBuffer += 8;

            // Parse the bool (as 1 byte) of whether to restake earnings.
            uint8_t restakeEarnings = dataBuffer[0];
            cx_hash((cx_hash_t *) &tx_state->hash, 0, dataBuffer, 1, NULL, 0);
            if (restakeEarnings == 0) {
                os_memmove(ctx->restake, "No\0", 3);
            } else if (restakeEarnings == 1) {
                os_memmove(ctx->restake, "Yes\0", 4);
            } else {
                THROW(SW_INVALID_TRANSACTION);
            }

            ux_flow_init(0, ux_sign_add_baker, NULL);
            *flags |= IO_ASYNCH_REPLY;
        } else {
            THROW(SW_INVALID_PARAM);
        }
    } else {
        THROW(SW_INVALID_STATE);
    }
}
