#include <os.h>

#include "sign.h"
#include "util.h"

static signUpdateValidatorScoreParametersContext_t *ctx = &global.signUpdateValidatorScoreParameters;
static tx_state_t *tx_state = &global_tx_state;

UX_STEP_NOCB(
    ux_sign_validator_score_parameters_type_step,
    bn,
    {"Update type", (char *) global.signUpdateValidatorScoreParameters.updateTypeText});
UX_STEP_NOCB(
    ux_sign_validator_score_parameters_1_step,
    bnnn_paging,
    {.title = "Max missed rounds", .text = (char *) global.signUpdateValidatorScoreParameters.max_missed_rounds});
UX_FLOW(
    ux_sign_validator_score_parameters,
    &ux_sign_flow_shared_review,
    &ux_sign_validator_score_parameters_type_step,
    &ux_sign_validator_score_parameters_1_step,
    &ux_sign_flow_shared_sign,
    &ux_sign_flow_shared_decline);

void handleSignUpdateValidatorScoreParameters(uint8_t *cdata, volatile unsigned int *flags) {
    cdata += parseKeyDerivationPath(cdata);

    cx_sha256_init(&tx_state->hash);
    cdata += hashUpdateHeaderAndType(cdata, UPDATE_TYPE_VALIDATOR_SCORE_PARAMETERS);

    // Set update type text for display
    strncpy(
        ctx->updateTypeText,
        getUpdateTypeText(UPDATE_TYPE_VALIDATOR_SCORE_PARAMETERS),
        sizeof(ctx->updateTypeText));

    // A 64-bit number representing the number of blocks a validator is allowed to miss before being flagged for
    // suspension.
    uint64_t max_missed_rounds = U8BE(cdata, 0);
    updateHash((cx_hash_t *) &tx_state->hash, cdata, 8);
    numberToText(ctx->max_missed_rounds, sizeof(ctx->max_missed_rounds), max_missed_rounds);

    ux_flow_init(0, ux_sign_validator_score_parameters, NULL);
    *flags |= IO_ASYNCH_REPLY;
}
