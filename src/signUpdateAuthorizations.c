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

UX_STEP_CB(
    ux_sign_update_authorizations_review_1_step,
    nn,
    sendSuccessNoIdle(),
    {
      "Update",
      (char *) global.signUpdateAuthorizations.type
    });
UX_FLOW(ux_sign_update_authorizations_review,
    &ux_sign_flow_shared_review,
    &ux_sign_update_authorizations_review_1_step
);

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

/**
 * Helper method for mapping an authorization type, i.e. the type of the access
 * structure currently being processed, to a display text that we can show to
 * the user in the UI.
 * 
 * Note: An error is thrown if this method is called with the AUTHORIZATION_END 
 * type, as that value is only used to register that all access structures have
 * been processed.
 */ 
const char* getAuthorizationName(authorizationType_e type) {
    switch (type) {
        case AUTHORIZATION_EMERGENCY: return "Emergency";
        case AUTHORIZATION_PROTOCOL: return "Protocol";
        case AUTHORIZATION_ELECTION_DIFFICULTY: return "Election difficulty";
        case AUTHORIZATION_EURO_PER_ENERGY: return "Euro per energy";
        case AUTHORIZATION_MICRO_GTU_PER_EURO: return "uGTU per Euro";
        case AUTHORIZATION_FOUNDATION_ACCOUNT: return "Foundation account";
        case AUTHORIZATION_MINT_DISTRIBUTION: return "Mint distribution";
        case AUTHORIZATION_TRANSACTION_FEE_DISTRIBUTION: return "Transaction fee distribution";
        case AUTHORIZATION_GAS_REWARDS: return "GAS rewards";
        case AUTHORIZATION_BAKER_STAKE_THRESHOLD: return "Baker stake threshold";
        case AUTHORIZATION_ADD_ANONYMITY_REVOKER: return "Add anonymity revoker";
        case AUTHORIZATION_ADD_IDENTITY_PROVIDER: return "Add identity provider";
        case AUTHORIZATION_END: THROW(SW_INVALID_STATE);
    }
}

/**
 * Method to be called when the user validates the threshold in the UI. If there are 
 * additional authorization types to process, then ask for additional data, otherwise 
 * continue to the signing flow as this marks the end of the transaction.
 */
void processThreshold() {
    ctx->authorizationType += 1;

    if (ctx->authorizationType == AUTHORIZATION_END) {
        ux_flow_init(0, ux_sign_flow_shared, NULL);
    } else {
        // Ask for the next access structure, as we have not processed all of 
        // them yet.
        ctx->state = TX_UPDATE_AUTHORIZATIONS_ACCESS_STRUCTURE_SIZE;
        sendSuccessNoIdle(); 
    }
}

// Cycle through the received key indices for the current access structure, and display
// it to the user. If we have completed processing the current access structure, then
// move on to receiving the threshold.
void processKeyIndices() {
    if (ctx->accessStructureSize == 0) {
        // The current access structure has been fully processed, continue to the threshold
        // for the current access structure.
        ctx->state = TX_UPDATE_AUTHORIZATIONS_ACCESS_STRUCTURE_THRESHOLD;
        sendSuccessNoIdle();
    } else if (ctx->processedCount == 0) {
        // The current batch was processed, but there are more to be processed. Ask for more data.
        sendSuccessNoIdle();
    } else {
        uint16_t keyIndex = U2BE(ctx->buffer, ctx->bufferPointer);
        bin2dec(ctx->displayKeyIndex, keyIndex);
        memmove(ctx->title, getAuthorizationName(ctx->authorizationType), 29);
        ctx->bufferPointer += 2;
        ctx->accessStructureSize -= 1;
        ctx->processedCount -= 1;
        ux_flow_init(0, ux_update_authorizations, NULL);
    }
}

#define P1_INITIAL                      0x00    // Contains key derivation path, update header and update kind, update key type and count of incoming public update keys.
#define P1_PUBLIC_KEY                   0x01    // Contains one public-key per command.
#define P1_ACCESS_STRUCTURE_INITIAL     0x02    // Contains the number of public-key indices for the current access.
#define P1_ACCESS_STRUCTURE             0x03    // Contains the public-key indices for the current access structure.
#define P1_ACCESS_STRUCTURE_THRESHOLD   0x04    // Contains the threshold for the current access structure.

void handleSignUpdateAuthorizations(uint8_t *cdata, uint8_t p1, uint8_t updateType, uint8_t dataLength, volatile unsigned int *flags) {
    if (p1 != P1_INITIAL && tx_state->initialized != true) {
        THROW(SW_INVALID_STATE);
    }

    if (p1 == P1_INITIAL && tx_state->initialized == false) {
        cdata += parseKeyDerivationPath(cdata);
        cx_sha256_init(&tx_state->hash);
        cdata += hashUpdateHeaderAndType(cdata, updateType);
        ctx->authorizationType = 0;
        tx_state->initialized = true;

        uint8_t keyUpdateType = cdata[0];
        if (keyUpdateType == ROOT_UPDATE_LEVEL_2) {
            memmove(ctx->type, "Level 2 w. root keys\0", 21);
        } else if (keyUpdateType == LEVEL1_UPDATE_LEVEL_2) {
            memmove(ctx->type, "Level 2 w. level 1 keys\0", 24);
        } else {
            THROW(SW_INVALID_TRANSACTION);
        }
        cx_hash((cx_hash_t *) &tx_state->hash, 0, cdata, 1, NULL, 0);
        cdata += 1;

        ctx->publicKeyListLength = U2BE(cdata, 0);
        cx_hash((cx_hash_t *) &tx_state->hash, 0, cdata, 2, NULL, 0);

        ctx->state = TX_UPDATE_AUTHORIZATIONS_PUBLIC_KEY;
        ux_flow_init(0, ux_sign_update_authorizations_review, NULL);
        *flags |= IO_ASYNCH_REPLY;
    } else if (p1 == P1_PUBLIC_KEY && ctx->state == TX_UPDATE_AUTHORIZATIONS_PUBLIC_KEY && ctx->publicKeyListLength > 0) {
        // Hash the schemeId
        cx_hash((cx_hash_t *) &tx_state->hash, 0, cdata, 1, NULL, 0);
        cdata += 1;

        uint8_t publicKeyInput[32];
        memmove(publicKeyInput, cdata, 32);
        toHex(publicKeyInput, 32, ctx->publicKey);
        cx_hash((cx_hash_t *) &tx_state->hash, 0, publicKeyInput, 32, NULL, 0);

        ctx->publicKeyListLength -= 1;
        if (ctx->publicKeyListLength == 0) {
            ctx->state = TX_UPDATE_AUTHORIZATIONS_ACCESS_STRUCTURE_SIZE;
        }
        ux_flow_init(0, ux_update_authorizations_public_key, NULL);
        *flags |= IO_ASYNCH_REPLY;
    } else if (p1 == P1_ACCESS_STRUCTURE_INITIAL && ctx->state == TX_UPDATE_AUTHORIZATIONS_ACCESS_STRUCTURE_SIZE) {
        cx_hash((cx_hash_t *) &tx_state->hash, 0, cdata, 2, NULL, 0);
        ctx->accessStructureSize = U2BE(cdata, 0);
        ctx->state = TX_UPDATE_AUTHORIZATIONS_ACCESS_STRUCTURE_INDEX;
        sendSuccessNoIdle();
    } else if (p1 == P1_ACCESS_STRUCTURE && ctx->state == TX_UPDATE_AUTHORIZATIONS_ACCESS_STRUCTURE_INDEX) {
        ctx->bufferPointer = 0;
        ctx->processedCount = 0;
        while (2 * ctx->processedCount < dataLength) {
            cx_hash((cx_hash_t *) &tx_state->hash, 0, cdata + (2 * ctx->processedCount), 2, NULL, 0);
            memmove(ctx->buffer + (2 * ctx->processedCount), cdata + (2 * ctx->processedCount), 2);
            ctx->processedCount += 1;
        }
        processKeyIndices();
        *flags |= IO_ASYNCH_REPLY;

    } else if (p1 == P1_ACCESS_STRUCTURE_THRESHOLD && ctx->state == TX_UPDATE_AUTHORIZATIONS_ACCESS_STRUCTURE_THRESHOLD) {
        uint16_t threshold = U2BE(cdata, 0);
        cx_hash((cx_hash_t *) &tx_state->hash, 0, cdata, 2, NULL, 0);
        bin2dec(ctx->displayKeyIndex, threshold);
        memmove(ctx->title, "Threshold", 10);

        ux_flow_init(0, ux_update_authorizations_threshold, NULL);
        *flags |= IO_ASYNCH_REPLY;
    } else {
        THROW(SW_INVALID_STATE);
    }
}
