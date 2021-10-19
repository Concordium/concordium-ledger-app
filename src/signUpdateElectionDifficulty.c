#include <os.h>
#include <os_io_seproxyhal.h>
#include "cx.h"
#include <stdint.h>
#include "menu.h"
#include "util.h"
#include <string.h>
#include "sign.h"
#include "responseCodes.h"

static signElectionDifficultyContext_t *ctx = &global.signElectionDifficulty;
static tx_state_t *tx_state = &global_tx_state;

UX_STEP_NOCB(
    ux_sign_election_difficulty_1_step,
    bnnn_paging,
    {
      .title = "Election difficulty",
      .text = (char *) global.signElectionDifficulty.electionDifficulty
    });
UX_FLOW(ux_sign_election_difficulty,
    &ux_sign_flow_shared_review,
    &ux_sign_election_difficulty_1_step,
    &ux_sign_flow_shared_sign,
    &ux_sign_flow_shared_decline
);

void handleSignUpdateElectionDifficulty(uint8_t *cdata, volatile unsigned int *flags) {
    cdata += parseKeyDerivationPath(cdata);;
    cx_sha256_init(&tx_state->hash);
    cdata += hashUpdateHeaderAndType(cdata, UPDATE_TYPE_ELECTION_DIFFICULTY);


    uint32_t numerator = U4BE(cdata, 0);
    if (numerator > 100000) {
        THROW(ERROR_INVALID_TRANSACTION);
    }

    // Hash numerator
    cx_hash((cx_hash_t *) &tx_state->hash, 0, cdata, 4, NULL, 0);

    // Display numerator as "numerator/100000"
    int numeratorLength = numberToText(ctx->electionDifficulty, sizeof(ctx->electionDifficulty), numerator);
    uint8_t fraction[8] = "/100000";
    memmove(ctx->electionDifficulty + numeratorLength, fraction, 8);

    ux_flow_init(0, ux_sign_election_difficulty, NULL);
    *flags |= IO_ASYNCH_REPLY;
}
