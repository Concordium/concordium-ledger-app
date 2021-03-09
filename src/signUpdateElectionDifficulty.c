#include <os.h>
#include <os_io_seproxyhal.h>
#include "cx.h"
#include <stdint.h>
#include "menu.h"
#include "util.h"
#include <string.h>
#include "sign.h"

static signElectionDifficultyContext_t *ctx = &global.signElectionDifficulty;
static tx_state_t *tx_state = &global_tx_state;

UX_STEP_NOCB(
    ux_sign_election_difficulty_0_step,
    nn,
    {
      "Review",
      "transaction"
    });
UX_STEP_NOCB(
    ux_sign_election_difficulty_1_step,
    bn_paging,
    {
      .title = "Election difficulty",
      .text = (char *) global.signElectionDifficulty.electionDifficulty
    });
UX_STEP_CB(
    ux_sign_election_difficulty_2_step,
    pnn,
    buildAndSignTransactionHash(),
    {
      &C_icon_validate_14,
      "Sign",
      "transaction"
    });
UX_STEP_CB(
    ux_sign_election_difficulty_3_step,
    pnn,
    declineToSignTransaction(),
    {
      &C_icon_crossmark,
      "Decline to",
      "sign transaction"
    });
UX_FLOW(ux_sign_election_difficulty,
    &ux_sign_election_difficulty_0_step,
    &ux_sign_election_difficulty_1_step,
    &ux_sign_election_difficulty_2_step,
    &ux_sign_election_difficulty_3_step
);

void handleSignUpdateElectionDifficulty(uint8_t *dataBuffer, volatile unsigned int *flags) {
    int bytesRead = parseKeyDerivationPath(dataBuffer);
    dataBuffer += bytesRead;

    cx_sha256_init(&tx_state->hash);

    // Add UpdateHeader to hash.
    cx_hash((cx_hash_t *) &tx_state->hash, 0, dataBuffer, UPDATE_HEADER_LENGTH, NULL, 0);
    dataBuffer += UPDATE_HEADER_LENGTH;

    // All update transactions are pre-pended by their type.
    uint8_t updateType = dataBuffer[0];
    if (updateType != 2) {
        THROW(SW_INVALID_TRANSACTION);
    }
    cx_hash((cx_hash_t *) &tx_state->hash, 0, dataBuffer, 1, NULL, 0);
    dataBuffer += 1;

    uint8_t fraction[10] = "/100000";
    uint32_t numerator = U4BE(dataBuffer, 0);
    if (numerator > 100000) {
        THROW(SW_INVALID_TRANSACTION);
    }
    int numeratorLength = bin2dec(ctx->electionDifficulty, numerator);
    cx_hash((cx_hash_t *) &tx_state->hash, 0, dataBuffer, 4, NULL, 0);
    dataBuffer += 4;
    os_memmove(ctx->electionDifficulty + numeratorLength, fraction, 10);

    ux_flow_init(0, ux_sign_election_difficulty, NULL);
    *flags |= IO_ASYNCH_REPLY;
}
