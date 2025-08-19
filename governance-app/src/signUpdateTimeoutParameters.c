#include <os.h>
#include <os_io_seproxyhal.h>
#include <stdint.h>
#include <string.h>

#include "cx.h"
#include "menu.h"
#include "responseCodes.h"
#include "sign.h"
#include "util.h"

static signUpdateTimeoutParametersContext_t *ctx = &global.signUpdateTimeoutParameters;
static tx_state_t *tx_state = &global_tx_state;

UX_STEP_NOCB(
    ux_sign_update_timeout_parameters_type_step,
    bn,
    {"Update type", (char *) global.signUpdateTimeoutParameters.updateTypeText});

UX_STEP_NOCB(
    ux_sign_update_timeout_parameters_1_step,
    bnnn_paging,
    {.title = "Timeout base", .text = (char *) global.signUpdateTimeoutParameters.timeoutBase});

UX_STEP_NOCB(
    ux_sign_update_timeout_parameters_2_step,
    bnnn_paging,
    {.title = "Increase ratio", .text = (char *) global.signUpdateTimeoutParameters.increaseTimeoutRatio});

UX_STEP_NOCB(
    ux_sign_update_timeout_parameters_3_step,
    bnnn_paging,
    {.title = "Decrease ratio", .text = (char *) global.signUpdateTimeoutParameters.decreaseTimeoutRatio});

UX_FLOW(
    ux_sign_update_timeout_parameters,
    &ux_sign_flow_shared_review,
    &ux_sign_update_timeout_parameters_type_step,
    &ux_sign_update_timeout_parameters_1_step,
    &ux_sign_update_timeout_parameters_2_step,
    &ux_sign_update_timeout_parameters_3_step,
    &ux_sign_flow_shared_sign,
    &ux_sign_flow_shared_decline);

void handleSignTimeoutParameters(uint8_t *cdata, volatile unsigned int *flags) {
    cdata += parseKeyDerivationPath(cdata);
    cx_sha256_init(&tx_state->hash);
    cdata += hashUpdateHeaderAndType(cdata, UPDATE_TYPE_TIMEOUT_PARAMETERS);

    // Set update type text
    strncpy(ctx->updateTypeText, getUpdateTypeText(UPDATE_TYPE_TIMEOUT_PARAMETERS), sizeof(ctx->updateTypeText));
    ctx->updateTypeText[sizeof(ctx->updateTypeText) - 1] = '\0';

    uint64_t timeoutBase = U8BE(cdata, 0);
    updateHash((cx_hash_t *) &tx_state->hash, cdata, 8);
    cdata += 8;
    numberToText(ctx->timeoutBase, sizeof(ctx->timeoutBase), timeoutBase);

    cdata += hashAndLoadU64Ratio(cdata, ctx->increaseTimeoutRatio, sizeof(ctx->increaseTimeoutRatio));
    hashAndLoadU64Ratio(cdata, ctx->decreaseTimeoutRatio, sizeof(ctx->decreaseTimeoutRatio));

    ux_flow_init(0, ux_sign_update_timeout_parameters, NULL);
    *flags |= IO_ASYNCH_REPLY;
}
