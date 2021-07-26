#include <os.h>
#include "util.h"
#include "sign.h"

static signUpdateKeysWithRootKeysContext_t *ctx = &global.signUpdateKeysWithRootKeysContext;
static tx_state_t *tx_state = &global_tx_state;

UX_STEP_CB(
    ux_sign_root_keys_review_1_step,
    nn,
    sendSuccessNoIdle(),
    {
      "Update",
      (char *) global.signUpdateKeysWithRootKeysContext.type
    });
UX_FLOW(ux_sign_root_keys_review,
    &ux_sign_flow_shared_review,
    &ux_sign_root_keys_review_1_step
);

UX_STEP_CB(
    ux_sign_root_keys_update_1_step,
    bnnn_paging,
    sendSuccessNoIdle(),
    {
      .title = "Update key",
      .text = (char *) global.signUpdateKeysWithRootKeysContext.updateVerificationKey
    });
UX_FLOW(ux_sign_root_keys_update_key,
    &ux_sign_root_keys_update_1_step
);

UX_STEP_NOCB(
    ux_sign_root_keys_update_threshold_0_step,
    bnnn_paging,
    {
      .title = "Threshold",
      .text = (char *) global.signUpdateKeysWithRootKeysContext.threshold
    });
UX_FLOW(ux_sign_root_keys_update_threshold,
    &ux_sign_root_keys_update_threshold_0_step,
    &ux_sign_flow_shared_sign,
    &ux_sign_flow_shared_decline
);

#define P1_INITIAL      0x00
#define P1_UPDATE_KEYS  0x01
#define P1_THRESHOLD    0x02

void handleSignHigherLevelKeys(uint8_t *cdata, uint8_t p1, uint8_t updateType, volatile unsigned int *flags) {
    if (p1 == P1_INITIAL && tx_state->initialized == false) {
        cdata += parseKeyDerivationPath(cdata);
        cx_sha256_init(&tx_state->hash);
        tx_state->initialized = true;
        cdata += hashUpdateHeaderAndType(cdata, updateType);

        // The specific type of key update is determined by this byte.
        uint8_t keyUpdateType = cdata[0];
        if (updateType == UPDATE_TYPE_UPDATE_ROOT_KEYS) {
            if (keyUpdateType == ROOT_UPDATE_ROOT) {
                memmove(ctx->type, "Root w. root keys\0", 18);   
            } else if (keyUpdateType == ROOT_UPDATE_LEVEL_1) {
                memmove(ctx->type, "Level 1 w. root keys\0", 21);
            } else {
                THROW(SW_INVALID_TRANSACTION);
            }
        } else if (updateType == UPDATE_TYPE_UPDATE_LEVEL1_KEYS) {
            if (keyUpdateType == ROOT_UPDATE_LEVEL_1) {
            } else if (keyUpdateType == LEVEL1_UPDATE_LEVEL_1) {
                memmove(ctx->type, "Level 1 w. level 1 keys\0", 25);
            } else {
                THROW(SW_INVALID_TRANSACTION);
            }
        }
        cx_hash((cx_hash_t *) &tx_state->hash, 0, cdata, 1, NULL, 0);
        cdata += 1;

        ctx->numberOfUpdateKeys = U2BE(cdata, 0);
        cx_hash((cx_hash_t *) &tx_state->hash, 0, cdata, 2, NULL, 0);
        cdata += 2;

        ctx->state = TX_UPDATE_KEYS_KEY;
        ux_flow_init(0, ux_sign_root_keys_review, NULL);
        *flags |= IO_ASYNCH_REPLY;
    } else if (p1 == P1_UPDATE_KEYS && ctx->state == TX_UPDATE_KEYS_KEY) {
        if (ctx->numberOfUpdateKeys <= 0) {
            // We have already received all the expected keys, so the receieved
            // transaction is invalid.
            THROW(SW_INVALID_TRANSACTION);
        }

        // Hash the schemeId
        cx_hash((cx_hash_t *) &tx_state->hash, 0, cdata, 1, NULL, 0);
        cdata += 1;

        toHex(cdata, 32, ctx->updateVerificationKey);
        cx_hash((cx_hash_t *) &tx_state->hash, 0, cdata, 32, NULL, 0);
        cdata += 32;

        ctx->numberOfUpdateKeys -= 1;
        if (ctx->numberOfUpdateKeys == 0) {
            ctx->state = TX_UPDATE_KEYS_THRESHOLD;
        }

        ux_flow_init(0, ux_sign_root_keys_update_key, NULL);
        *flags |= IO_ASYNCH_REPLY;
    } else if (p1 == P1_THRESHOLD && ctx->state == TX_UPDATE_KEYS_THRESHOLD) {
        uint16_t threshold = U2BE(cdata, 0);
        bin2dec(ctx->threshold, threshold);
        cx_hash((cx_hash_t *) &tx_state->hash, 0, cdata, 2, NULL, 0);
        cdata += 2;

        ux_flow_init(0, ux_sign_root_keys_update_threshold, NULL);
        *flags |= IO_ASYNCH_REPLY;
    } else {
        THROW(SW_INVALID_STATE);
    }
}
