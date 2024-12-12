#include "globals.h"

static deployModule_t *ctx_deploy_module = &global.deployModule;
static tx_state_t *tx_state = &global_tx_state;

#define P1_INITIAL 0x00
#define P1_SOURCE  0x01

void handleDeployModule(uint8_t *cdata, uint8_t p1, uint8_t lc) {
    if (p1 == P1_INITIAL) {
        cdata += parseKeyDerivationPath(cdata);
        cx_sha256_init(&tx_state->hash);
        cdata += hashAccountTransactionHeaderAndKind(cdata, DEPLOY_MODULE);

        // hash the version and source length
        updateHash((cx_hash_t *)&tx_state->hash, cdata, 8);
        ctx_deploy_module->version = U4BE(cdata, 0);
        ctx_deploy_module->sourceLength = U4BE(cdata, 4);
        ctx_deploy_module->remainingSourceLength = ctx_deploy_module->sourceLength;
        // TODO: Format the version
        numberToText((uint8_t *)ctx_deploy_module->versionDisplay,
                     sizeof(ctx_deploy_module->versionDisplay),
                     ctx_deploy_module->version);
        sendSuccessNoIdle();
    }

    else if (p1 == P1_SOURCE && ctx_deploy_module->remainingSourceLength > 0) {
        if (ctx_deploy_module->remainingSourceLength < lc) {
            THROW(ERROR_INVALID_SOURCE_LENGTH);
        }

        updateHash((cx_hash_t *)&tx_state->hash, cdata, lc);
        ctx_deploy_module->remainingSourceLength -= lc;
        if (ctx_deploy_module->remainingSourceLength > 0) {
            sendSuccessNoIdle();
        } else if (ctx_deploy_module->remainingSourceLength == 0) {
            uiDeployModuleDisplay();
        }

    } else {
        THROW(ERROR_INVALID_STATE);
    }
}