#include "os.h"
#include "ux.h"
#include "globals.h"
#include "util.h"
#include "menu.h"

static accountSubtreePath_t *keyPath = &path;

// The public-key is 32 bytes, and when converted to hexadecimal it takes up 64 characters. The 65th character
// is for the C string terminator '\0'.
static char publicKeyAsHex[65];

// UI definitions for the comparison of the public-key. The public-key should be displayed in the wallet, and then
// compared with the public-key being displayed on the Ledger device. This is done to avoid the computer maliciously
// generating a public-key for another account than expected, as the user can validate that they are equal and
// escape the flow in the wallet if they are inequal.
UX_STEP_VALID(
    ux_compare_public_flow_0_step,
    bnnn_paging,
    ui_idle(),
    {
        .title = "Public-key",
        .text = publicKeyAsHex,
    });
UX_FLOW(ux_compare_public_flow,
  &ux_compare_public_flow_0_step
);

// Derive the public-key for the given address, convert it to hex (human readable), and then write it to
// the APDU buffer to be returned to the computer. Continue into the comparison/verification UI flow after
// having returned the public-key to the computer.
void sendPublicKey() {
    uint8_t publicKey[32];

    getPublicKey(keyPath->identity, keyPath->accountIndex, publicKey);
    toHex(publicKey, sizeof(publicKey), publicKeyAsHex);

    // tx is holding the offset in the buffer we have written to. It is a convention to call this tx for ledger apps.
    uint8_t tx = 0;

    // Write the public-key as hex to the APDU buffer.
    for (uint8_t i = 0; i < sizeof(publicKeyAsHex); i++) {
        G_io_apdu_buffer[i] = publicKeyAsHex[i];
        tx++;
    }

    // Send back success response including the public-key.
    sendSuccessNoIdle(tx);

    // Goto the comparison UX flow where the user can compare the public-key on the computer with the public-key
    // displayed on the device.
    ux_flow_init(0, ux_compare_public_flow, NULL);
}

// UI definitions for the approval of the generation of a public-key. This prompts the user to accept that
// a public-key will be generated and returned to the computer for comparison in the next step.
UX_STEP_VALID(
    ux_generate_public_flow_0_step,
    pnn,
    sendPublicKey(),
    {
      &C_icon_validate_14,
      "Generate",
      "Public-key?"
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

// Entry-point from the main class to the handler of public keys.
void handleGetPublicKey(uint8_t *dataBuffer, volatile unsigned int *flags) {
    parseAccountSignatureKeyPath(dataBuffer);
    dataBuffer += 2;

    // Display the UI for the public-key flow, where the user has to compare the public-key displayed on the device
    // and the public-key shown in the wallet.
    ux_flow_init(0, ux_generate_public_flow, NULL);

    // Tell the main process to wait for a button press.
    *flags |= IO_ASYNCH_REPLY;
}