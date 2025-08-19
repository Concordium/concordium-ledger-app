#include <os.h>
#include <os_io_seproxyhal.h>
#include <stdint.h>
#include <string.h>

#include "cx.h"
#include "menu.h"
#include "responseCodes.h"
#include "sign.h"
#include "util.h"

static signUpdateFinalizationCommitteeParametersContext_t *ctx = &global.signUpdateFinalizationCommitteeParameters;
static tx_state_t *tx_state = &global_tx_state;

UX_STEP_NOCB(
    ux_sign_update_finalization_committee_parameters_type_step,
    bn,
    {"Update type", (char *) global.signUpdateFinalizationCommitteeParameters.updateTypeText});

UX_STEP_NOCB(
    ux_sign_update_finalization_committee_parameters_step_1,
    bnnn_paging,
    {.title = "Min finalizers", .text = (char *) global.signUpdateFinalizationCommitteeParameters.minFinalizers});

UX_STEP_NOCB(
    ux_sign_update_finalization_committee_parameters_step_2,
    bnnn_paging,
    {.title = "Max finalizers", .text = (char *) global.signUpdateFinalizationCommitteeParameters.maxFinalizers});

UX_STEP_NOCB(
    ux_sign_update_finalization_committee_parameters_step_3,
    bnnn_paging,
    {.title = "Stake threshold",
     .text = (char *) global.signUpdateFinalizationCommitteeParameters.relativeStakeThreshold});

UX_FLOW(
    ux_sign_update_finalization_committee_parameters,
    &ux_sign_flow_shared_review,
    &ux_sign_update_finalization_committee_parameters_type_step,
    &ux_sign_update_finalization_committee_parameters_step_1,
    &ux_sign_update_finalization_committee_parameters_step_2,
    &ux_sign_update_finalization_committee_parameters_step_3,
    &ux_sign_flow_shared_sign,
    &ux_sign_flow_shared_decline);

void handleSignUpdateFinalizationCommitteeParameters(uint8_t *cdata, volatile unsigned int *flags) {
    cdata += parseKeyDerivationPath(cdata);
    cx_sha256_init(&tx_state->hash);
    cdata += hashUpdateHeaderAndType(cdata, UPDATE_TYPE_FINALIZATION_COMMITTEE_PARAMETERS);

    // Set update type text
    strncpy(
        ctx->updateTypeText,
        getUpdateTypeText(UPDATE_TYPE_FINALIZATION_COMMITTEE_PARAMETERS),
        sizeof(ctx->updateTypeText));
    ctx->updateTypeText[sizeof(ctx->updateTypeText) - 1] = '\0';

    uint32_t minFinalizers = U4BE(cdata, 0);
    uint32_t maxFinalizers = U4BE(cdata, 4);
    uint32_t relativeStakeThreshold = U4BE(cdata, 8);

    updateHash((cx_hash_t *) &tx_state->hash, cdata, 12);

    bin2dec(ctx->minFinalizers, sizeof(ctx->minFinalizers), minFinalizers);
    bin2dec(ctx->maxFinalizers, sizeof(ctx->maxFinalizers), maxFinalizers);
    fractionToPercentageDisplay(
        ctx->relativeStakeThreshold,
        sizeof(ctx->relativeStakeThreshold),
        relativeStakeThreshold);

    ux_flow_init(0, ux_sign_update_finalization_committee_parameters, NULL);
    *flags |= IO_ASYNCH_REPLY;
}
