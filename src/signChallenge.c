#include <os.h>
#include <os_io_seproxyhal.h>
#include "cx.h"
#include <stdint.h>
#include "menu.h"
#include "util.h"
#include <string.h>
#include "base58check.h"
#include "sign.h"

static tx_state_t *tx_state = &global_tx_state;

UX_STEP_CB(
    ux_sign_credential_challenge_flow_0_step,
    pnn,
    buildAndSignTransactionHash(),
    {
      &C_icon_validate_14,
      "Sign account",
      "challenge"
    });
UX_STEP_CB(
    ux_sign_credential_challenge_flow_1_step,
    pnn,
    declineToSignTransaction(),
    {
      &C_icon_crossmark,
      "Decline to",
      "sign challenge"
    });
UX_FLOW(ux_sign_credential_challenge_flow,
    &ux_sign_credential_challenge_flow_0_step,
    &ux_sign_credential_challenge_flow_1_step
);

void handleSignChallenge(uint8_t *dataBuffer, volatile unsigned int *flags) {
    int bytesRead = parseKeyDerivationPath(dataBuffer);
    dataBuffer += bytesRead;

    // The challenge is 32 bytes, but we cannot display it to the user as the user has no
    // way of validating it as being correct.
    cx_sha256_init(&tx_state->hash);
    cx_hash((cx_hash_t *) &tx_state->hash, 0, dataBuffer, 32, NULL, 0);

    ux_flow_init(0, ux_sign_credential_challenge_flow, NULL);
    *flags |= IO_ASYNCH_REPLY;
}
