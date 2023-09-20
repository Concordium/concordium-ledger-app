#include <os.h>
#include <os_io_seproxyhal.h>
#include <stdint.h>
#include <string.h>

#include "cx.h"
#include "menu.h"
#include "responseCodes.h"
#include "sign.h"
#include "util.h"

static signUpdateBlockEnergyLimitContext_t *ctx = &global.signUpdateBlockEnergyLimit;
static tx_state_t *tx_state = &global_tx_state;

UX_STEP_NOCB(
    ux_sign_update_block_energy_limit_1_step,
    bnnn_paging,
    {.title = "Block energy limit", .text = (char *) global.signUpdateBlockEnergyLimit.blockEnergyLimit});

UX_FLOW(
    ux_sign_update_block_energy_limit,
    &ux_sign_flow_shared_review,
    &ux_sign_update_block_energy_limit_1_step,
    &ux_sign_flow_shared_sign,
    &ux_sign_flow_shared_decline);

void handleSignUpdateBlockEnergyLimit(uint8_t *cdata, volatile unsigned int *flags) {
    cdata += parseKeyDerivationPath(cdata);
    cx_sha256_init(&tx_state->hash);
    cdata += hashUpdateHeaderAndType(cdata, UPDATE_TYPE_BLOCK_ENERGY_LIMIT);

    uint64_t blockEnergyLimit = U8BE(cdata, 0);
    updateHash((cx_hash_t *) &tx_state->hash, cdata, 8);
    cdata += 8;
    numberToText(ctx->blockEnergyLimit, sizeof(ctx->blockEnergyLimit), blockEnergyLimit);

    ux_flow_init(0, ux_sign_update_block_energy_limit, NULL);
    *flags |= IO_ASYNCH_REPLY;
}
