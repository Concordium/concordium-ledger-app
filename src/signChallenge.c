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
static signAccountChallenge_t *ctx = &global.signAccountChallengeContext;

void signChallenge();

UX_STEP_CB(
    ux_sign_credential_challenge_flow_0_step,
    pnn,
    signChallenge(),
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

    // Keep the challenge bytes so they can be signed. It does not make sense to
    // display the value to the user, as they have no of validating it.
    os_memmove(ctx->challenge, dataBuffer, 32);

    ux_flow_init(0, ux_sign_credential_challenge_flow, NULL);
    *flags |= IO_ASYNCH_REPLY;
}

void signChallenge() {
    uint8_t signedChallenge[64];

    // Note that it is not a hash being signed here, so perhaps the naming of that
    // method should be generalized. In this case it's just the raw challenge bytes.
    signTransactionHash(ctx->challenge, signedChallenge);

    // Send the signature back to the user.
    os_memmove(G_io_apdu_buffer, signedChallenge, sizeof(signedChallenge));
    sendSuccess(sizeof(signedChallenge));
}
