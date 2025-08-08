#include <os.h>
#include <os_io_seproxyhal.h>
#include <stdint.h>
#include <string.h>

#include "cx.h"
#include "menu.h"
#include "responseCodes.h"
#include "sign.h"
#include "util.h"

static signUpdateMinBlockTimeContext_t *ctx = &global.signUpdateMinBlockTime;
static tx_state_t *tx_state = &global_tx_state;

UX_STEP_NOCB(
    ux_sign_update_min_block_time_type_step,
    bn,
    {"Update type", (char *) global.signUpdateMinBlockTime.updateTypeText});
UX_STEP_NOCB(
    ux_sign_update_min_block_time_1_step,
    bnnn_paging,
    {.title = "Min block time", .text = (char *) global.signUpdateMinBlockTime.minBlockTime});

UX_FLOW(
    ux_sign_update_min_block_time,
    &ux_sign_flow_shared_review,
    &ux_sign_update_min_block_time_type_step,
    &ux_sign_update_min_block_time_1_step,
    &ux_sign_flow_shared_sign,
    &ux_sign_flow_shared_decline);

void handleSignMinBlockTime(uint8_t *cdata, volatile unsigned int *flags) {
    cdata += parseKeyDerivationPath(cdata);
    cx_sha256_init(&tx_state->hash);
    cdata += hashUpdateHeaderAndType(cdata, UPDATE_TYPE_MIN_BLOCK_TIME);

    strncpy(ctx->updateTypeText, getUpdateTypeText(UPDATE_TYPE_MIN_BLOCK_TIME), sizeof(ctx->updateTypeText));

    uint64_t minBlockTime = U8BE(cdata, 0);
    updateHash((cx_hash_t *) &tx_state->hash, cdata, 8);
    cdata += 8;
    numberToTextWithUnit(ctx->minBlockTime, sizeof(ctx->minBlockTime), minBlockTime, (uint8_t*)"ms", 2);

    ux_flow_init(0, ux_sign_update_min_block_time, NULL);
    *flags |= IO_ASYNCH_REPLY;
}
