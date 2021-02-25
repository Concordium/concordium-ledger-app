#include "os.h"
#include "ux.h"
#include "globals.h"
#include "util.h"
#include "menu.h"

static keyDerivationPath_t *keyPath = &path;
static exportPublicKeyContext_t *ctx = &global.exportPublicKeyContext;

void sendPublicKey();

// UI definitions for the approval of the generation of a public-key. This prompts the user to accept that
// a public-key will be generated and returned to the computer.
UX_STEP_VALID(
    ux_generate_public_flow_0_step,
    pnn,
    sendPublicKey(),
    {
      &C_icon_validate_14,
      "Public-key",
      (char *) global.exportPublicKeyContext.display
    });
UX_STEP_VALID(
    ux_generate_public_flow_1_step,
    pb,
    sendUserRejection(),
    {
      &C_icon_crossmark,
      "Decline"
    });
UX_FLOW(ux_generate_public_flow,
    &ux_generate_public_flow_0_step,
    &ux_generate_public_flow_1_step,
    FLOW_LOOP
);

// Derive the public-key for the given address, convert it to hex (human readable), and then write it to
// the APDU buffer to be returned to the computer.
void sendPublicKey() {
    uint8_t publicKey[32];
    getPublicKey(publicKey);

    // tx is holding the offset in the buffer we have written to. It is a convention to call this tx for ledger apps.
    uint8_t tx = 0;

    // Write the public-key to the APDU buffer.
    for (uint8_t i = 0; i < sizeof(publicKey); i++) {
        G_io_apdu_buffer[i] = publicKey[i];
        tx++;
    }

    // Send back success response including the public-key.
    sendSuccess(tx);
}

// Entry-point from the main class to the handler of public keys.
void handleGetPublicKey(uint8_t *dataBuffer, volatile unsigned int *flags) {
    int bytesRead = parseKeyDerivationPath(dataBuffer);
    dataBuffer += bytesRead;

    getIdentityAccountDisplay(ctx->display);

    // Display the UI for the public-key flow, where the user can validate that the
    // public-key being generated is for the expected identity and account.
    ux_flow_init(0, ux_generate_public_flow, NULL);

    // Tell the main process to wait for a button press.
    *flags |= IO_ASYNCH_REPLY;
}