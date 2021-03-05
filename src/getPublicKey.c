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

    if (ctx->signPublicKey) {
        uint8_t signedPublicKey[64];
        // Note that it is not a hash being signed here, so perhaps the naming of that
        // method should be generalized. In this case it's the public-key.
        signTransactionHash(publicKey, signedPublicKey);
        
        for (uint8_t i = 32; i < sizeof(signedPublicKey) + 32; i++) {
            G_io_apdu_buffer[i] = signedPublicKey[i];
            tx++;
        }
    }

    // Send back success response including the public-key (and signature, if wanted).
    sendSuccess(tx);
}

// Entry-point from the main class to the handler of public keys.
void handleGetPublicKey(uint8_t *dataBuffer, uint8_t p1, uint8_t p2, volatile unsigned int *flags) {
    int bytesRead = parseKeyDerivationPath(dataBuffer);
    dataBuffer += bytesRead;

    // If P1 == 02, then the public-key is signed by its corresponding private key, and
    // appended to the returned public-key.
    ctx->signPublicKey = p2 == 1;

    // If P1 == 01, then we skip displaying the key being exported. This is used when it 
    // it is not important for the user to validate the key path, i.e. for governance, 
    // where an unauthorized key will be rejected anyway.
    if (p1 == 0x01) {
        sendPublicKey();
    } else {
        // If the key path is of length 5, then it is a request for a governance key.
        if (keyPath->pathLength == 5) {
            uint32_t purpose = keyPath->rawKeyDerivationPath[3];

            switch (purpose) {
                case 0:
                    os_memmove(ctx->display, "Gov. root", 10);
                    break;
                case 1:
                    os_memmove(ctx->display, "Gov. level 1", 13);
                    break;
                case 2:
                    os_memmove(ctx->display, "Gov. level 2", 13);
                    break;
                default:
                    THROW(SW_INVALID_PATH);
            }
        } else {
            getIdentityAccountDisplay(ctx->display);
        }

        // Display the UI for the public-key flow, where the user can validate that the
        // public-key being generated is the expected one.
        ux_flow_init(0, ux_generate_public_flow, NULL);

        // Tell the main process to wait for a button press.
        *flags |= IO_ASYNCH_REPLY;
    }
}
