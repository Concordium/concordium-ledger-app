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

// TODO move to shared memory
static int indexCounter;

void moveIndexPointer();
void processThreshold();

UX_STEP_CB(
    ux_update_authorizations_0_step,
    nn,
    moveIndexPointer(),
    {
      (char *) global.signUpdateAuthorizations.title,
      (char *) global.signUpdateAuthorizations.displayKeyIndex
    });
UX_FLOW(ux_update_authorizations,
    &ux_update_authorizations_0_step
);

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

UX_STEP_CB(
    ux_update_authorizations_public_key_0_step,
    bn_paging,
    sendSuccessNoIdle(0),
    {
        "Public-key",
        (char *) global.signUpdateAuthorizations.publicKey
    });
UX_FLOW(ux_update_authorizations_public_key,
    &ux_update_authorizations_public_key_0_step
);

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

void processThreshold() {
    if (ctx->authorizationType == END) {
        ux_flow_init(0, ux_sign_flow_shared, NULL);
    } else {
        sendSuccessNoIdle(0); // Ask for the next access structure, as we are not done processing all of them
    }
}

void moveIndexPointer() {
    if (ctx->accessStructureSize == 0) {
        ctx->authorizationType += 1;
        sendSuccessNoIdle(0);
    } else {
        uint16_t keyIndex = U2BE(ctx->buffer, indexCounter);
        bin2dec(ctx->displayKeyIndex, keyIndex);
        indexCounter += 2;
        os_memmove(ctx->title, getAuthorizationName(ctx->authorizationType), 20);
        ctx->accessStructureSize -= 1;
        ux_flow_init(0, ux_update_authorizations, NULL);
    }
}

#define P1_INITIAL                      0x00    // Contains key derivation path, update header and update kind.
#define P1_PUBLIC_KEY_INITIAL           0x01    // Contains the number of public-keys to update authorizations with.
#define P1_PUBLIC_KEY                   0x02    // Contains one public-key per command.
#define P1_ACCESS_STRUCTURE_INITIAL     0x03    // Contains the number of public-key indices for the current access structure.
#define P1_ACCESS_STRUCTURE             0x04
#define P1_ACCESS_STRUCTURE_THRESHOLD   0x05

void handleSignUpdateAuthorizations(uint8_t *dataBuffer, uint8_t p1, volatile unsigned int *flags) {
    if (p1 == P1_INITIAL) {
        int bytesRead = parseKeyDerivationPath(dataBuffer);
        dataBuffer += bytesRead;
        ctx->authorizationType = 0;

        cx_sha256_init(&tx_state->hash);

        // Hash update header and update kind.
        cx_hash((cx_hash_t *) &tx_state->hash, 0, dataBuffer, UPDATE_HEADER_LENGTH + 1, NULL, 0);
        dataBuffer += UPDATE_HEADER_LENGTH + 1;

        sendSuccessNoIdle(0);

    } else if (p1 == P1_PUBLIC_KEY_INITIAL) {
        ctx->publicKeyListLength = U2BE(dataBuffer, 0);
        cx_hash((cx_hash_t *) &tx_state->hash, 0, dataBuffer, 2, NULL, 0);
        sendSuccessNoIdle(0);

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
        sendSuccessNoIdle(0);

    } else if (p1 == P1_ACCESS_STRUCTURE) {
        indexCounter = 0;
        uint8_t processedCount = 0;
        while (ctx->accessStructureSize > 0 && processedCount < 127) {
            cx_hash((cx_hash_t *) &tx_state->hash, 0, dataBuffer + (2 * processedCount), 2, NULL, 0);
            os_memmove(ctx->buffer + (2 * processedCount), dataBuffer + (2 * processedCount), 2);
            processedCount += 1;
        }
        moveIndexPointer();
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
