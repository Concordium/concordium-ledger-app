#include <os.h>
#include <os_io_seproxyhal.h>
#include "cx.h"
#include <stdint.h>
#include "menu.h"
#include "util.h"
#include <string.h>
#include "sign.h"

static signUpdateAuthorizations_t *ctx = &global.signUpdateAuthorizations;
static tx_state_t *tx_state = &global_tx_state;

void processKeyIndices();
void processThreshold();

// UI for displaying access structure key indices. It displays the type of access
// structure and the key index to be authorized.
UX_STEP_CB(
    ux_update_authorizations_0_step,
    nn,
    processKeyIndices(),
    {
      (char *) global.signUpdateAuthorizations.title,
      (char *) global.signUpdateAuthorizations.displayKeyIndex
    });
UX_FLOW(ux_update_authorizations,
    &ux_update_authorizations_0_step
);

// UI for displaying the key threshold for an access structure.
UX_STEP_CB(
    ux_update_authorizations_threshold_0_step,
    nn,
    processThreshold(),
    {
      (char *) global.signUpdateAuthorizations.title,
      (char *) global.signUpdateAuthorizations.displayKeyIndex
    });
UX_FLOW(ux_update_authorizations_threshold,
    &ux_update_authorizations_threshold_0_step
);

// UI for displaying the list of public-keys that are being authorized.
UX_STEP_CB(
    ux_update_authorizations_public_key_0_step,
    bn_paging,
    sendSuccessNoIdle(),
    {
        "Public-key",
        (char *) global.signUpdateAuthorizations.publicKey
    });
UX_FLOW(ux_update_authorizations_public_key,
    &ux_update_authorizations_public_key_0_step
);

// Helper method for mapping authorization types, i.e. the type of the access
// structure currently being processed, to a display text. 
const char* getAuthorizationName(authorizationType_e type) {
    switch (type) {
        case EMERGENCY: return "Emergency";
        case AUTHORIZATION: return "Authorization";
        case PROTOCOL: return "Protocol";
        case ELECTION_DIFFICULTY: return "Election difficulty";
        case EURO_PER_ENERGY: return "Euro per energy";
        case MICRO_GTU_PER_EURO: return "uGTU per Euro";
        case END: return "END";
    }
}

// Method called when on the threshold display UI. If there are more authorization
// types to process, then ask the computer for additional data. If there are no 
// more types, then continue to the signing flow as this marks the end of the transaction.
void processThreshold() {
    if (ctx->authorizationType == END) {
        ux_flow_init(0, ux_sign_flow_shared, NULL);
    } else {
        sendSuccessNoIdle(); // Ask for the next access structure, as we are not done processing all of them
    }
}

// Cycle through the received key indices for the current access structure, and display
// it to the user. If we have completed processing the current access structure, then
// move on to the next authorization type.
void processKeyIndices() {
    if (ctx->accessStructureSize == 0) {
        // The current access structure has been fully processed, continue to the next.
        ctx->authorizationType += 1;
        sendSuccessNoIdle();
    } else if (ctx->processedCount == 0) {
        // The current batch was processed, but there are more to be processed. Ask for more data.
        sendSuccessNoIdle();
    } else {
        uint16_t keyIndex = U2BE(ctx->buffer, ctx->bufferPointer);
        bin2dec(ctx->displayKeyIndex, keyIndex);
        os_memmove(ctx->title, getAuthorizationName(ctx->authorizationType), 20);
        ctx->bufferPointer += 2;
        ctx->accessStructureSize -= 1;
        ctx->processedCount -= 1;
        ux_flow_init(0, ux_update_authorizations, NULL);
    }
}

#define P1_INITIAL                      0x00    // Contains key derivation path, update header and update kind.
#define P1_PUBLIC_KEY_INITIAL           0x01    // Contains the number of public-keys to update authorizations with.
#define P1_PUBLIC_KEY                   0x02    // Contains one public-key per command.
#define P1_ACCESS_STRUCTURE_INITIAL     0x03    // Contains the number of public-key indices for the current access.
#define P1_ACCESS_STRUCTURE             0x04    // Contains the public-key indices for the current access structure.
#define P1_ACCESS_STRUCTURE_THRESHOLD   0x05    // Contains the threshold for the current access structure.

void handleSignUpdateAuthorizations(uint8_t *dataBuffer, uint8_t p1, volatile unsigned int *flags) {
    if (p1 == P1_INITIAL) {
        int bytesRead = parseKeyDerivationPath(dataBuffer);
        dataBuffer += bytesRead;
        ctx->authorizationType = 0;

        cx_sha256_init(&tx_state->hash);

        // Hash update header and update kind.
        cx_hash((cx_hash_t *) &tx_state->hash, 0, dataBuffer, UPDATE_HEADER_LENGTH + 1, NULL, 0);
        dataBuffer += UPDATE_HEADER_LENGTH + 1;

        sendSuccessNoIdle();

    } else if (p1 == P1_PUBLIC_KEY_INITIAL) {
        ctx->publicKeyListLength = U2BE(dataBuffer, 0);
        cx_hash((cx_hash_t *) &tx_state->hash, 0, dataBuffer, 2, NULL, 0);
        sendSuccessNoIdle();

    } else if (p1 == P1_PUBLIC_KEY) {
        uint8_t publicKeyInput[32];
        os_memmove(publicKeyInput, dataBuffer, 32);
        cx_hash((cx_hash_t *) &tx_state->hash, 0, publicKeyInput, 32, NULL, 0);

        toHex(publicKeyInput, 32, ctx->publicKey);
        ux_flow_init(0, ux_update_authorizations_public_key, NULL);
        *flags |= IO_ASYNCH_REPLY;

    } else if (p1 == P1_ACCESS_STRUCTURE_INITIAL) {
        cx_hash((cx_hash_t *) &tx_state->hash, 0, dataBuffer, 2, NULL, 0);
        ctx->accessStructureSize = U2BE(dataBuffer, 0);
        sendSuccessNoIdle();

    } else if (p1 == P1_ACCESS_STRUCTURE) {
        ctx->bufferPointer = 0;
        ctx->processedCount = 0;

        // FIXME: This could be made more elegant, by determining whether we have a full batch, or it 
        // is the final batch with less than 127 key indices. That way the loop can be removed.
        while (ctx->accessStructureSize > 0 && ctx->processedCount < 127) {
            cx_hash((cx_hash_t *) &tx_state->hash, 0, dataBuffer + (2 * ctx->processedCount), 2, NULL, 0);
            os_memmove(ctx->buffer + (2 * ctx->processedCount), dataBuffer + (2 * ctx->processedCount), 2);
            ctx->processedCount += 1;
        }
        processKeyIndices();
        *flags |= IO_ASYNCH_REPLY;

    } else if (p1 == P1_ACCESS_STRUCTURE_THRESHOLD) {
        uint16_t threshold = U2BE(dataBuffer, 0);
        cx_hash((cx_hash_t *) &tx_state->hash, 0, dataBuffer, 2, NULL, 0);
        bin2dec(ctx->displayKeyIndex, threshold);
        os_memmove(ctx->title, "Threshold", 10);
        ux_flow_init(0, ux_update_authorizations_threshold, NULL);
        *flags |= IO_ASYNCH_REPLY;
    }
}
