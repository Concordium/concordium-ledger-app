#ifndef _CONCORDIUM_APP_ELECTION_DIFFICULTY_H_
#define _CONCORDIUM_APP_ELECTION_DIFFICULTY_H_

/**
 * Handles the signing flow, including updating the display, for the 'update election difficulty'
 * update instruction.
 * @param cdata please see /doc/ins_update_election_difficulty.md for details
 */
void handleSignUpdateElectionDifficulty(uint8_t *cdata, volatile unsigned int *flags);

typedef struct {
    uint8_t electionDifficulty[8];
} signElectionDifficultyContext_t;

#endif
