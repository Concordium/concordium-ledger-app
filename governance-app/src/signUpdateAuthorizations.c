#include <stdint.h>
#include <string.h>

#include "cx.h"
#include "globals.h"
#include "menu.h"
#include "responseCodes.h"
#include "sign.h"
#include "signHigherLevelKeyUpdate.h"
#include "util.h"

static signUpdateAuthorizations_t *ctx = &global.signUpdateAuthorizations;
static tx_state_t *tx_state = &global_tx_state;

void processKeyIndices(void);
void processThreshold(void);

UX_STEP_CB(
    ux_sign_update_authorizations_review_1_step,
    nn,
    sendSuccessNoIdle(),
    {"Update", (char *) global.signUpdateAuthorizations.type});
UX_FLOW(
    ux_sign_update_authorizations_review,
    &ux_sign_flow_shared_review,
    &ux_sign_update_authorizations_review_1_step);

// UI for displaying access structure key indices. It displays the type of access
// structure and the key index to be authorized.
UX_STEP_CB(
    ux_update_authorizations_0_step,
    nn,
    processKeyIndices(),
    {(char *) global.signUpdateAuthorizations.title, (char *) global.signUpdateAuthorizations.displayKeyIndex});
UX_FLOW(ux_update_authorizations, &ux_update_authorizations_0_step);

// UI for displaying the key threshold for an access structure.
UX_STEP_CB(
    ux_update_authorizations_threshold_0_step,
    nn,
    processThreshold(),
    {(char *) global.signUpdateAuthorizations.title, (char *) global.signUpdateAuthorizations.displayKeyIndex});
UX_FLOW(ux_update_authorizations_threshold, &ux_update_authorizations_threshold_0_step);

// UI for displaying the list of public-keys that are being authorized.
UX_STEP_CB(
    ux_update_authorizations_public_key_0_step,
    bnnn_paging,
    sendSuccessNoIdle(),
    {"Public key", (char *) global.signUpdateAuthorizations.publicKey});
UX_FLOW(ux_update_authorizations_public_key, &ux_update_authorizations_public_key_0_step);

/**
 * Helper method for mapping an authorization type, i.e. the type of the access
 * structure currently being processed, to a display text that we can show to
 * the user in the UI.
 *
 * Note: An error is thrown if this method is called with the AUTHORIZATION_END
 * type, as that value is only used to register that all access structures have
 * been processed.
 */
const char *getAuthorizationName(uint8_t type) {
    switch (type) {
        case AUTHORIZATION_EMERGENCY:
            return "Emergency";
        case AUTHORIZATION_PROTOCOL:
            return "Protocol";
        case AUTHORIZATION_CONSENSUS_PARAMETERS:
            return "Consensus";
        case AUTHORIZATION_EURO_PER_ENERGY:
            return "Euro per energy";
        case AUTHORIZATION_MICRO_GTU_PER_EURO:
            return "uCCD per Euro";
        case AUTHORIZATION_FOUNDATION_ACCOUNT:
            return "Foundation account";
        case AUTHORIZATION_MINT_DISTRIBUTION:
            return "Mint distribution";
        case AUTHORIZATION_TRANSACTION_FEE_DISTRIBUTION:
            return "Transaction fee distribution";
        case AUTHORIZATION_GAS_REWARDS:
            return "GAS rewards";
        case AUTHORIZATION_BAKER_STAKE_THRESHOLD:
            return "Baker stake threshold";
        case AUTHORIZATION_ADD_ANONYMITY_REVOKER:
            return "Add anonymity revoker";
        case AUTHORIZATION_ADD_IDENTITY_PROVIDER:
            return "Add identity provider";
        case AUTHORIZATION_COOLDOWN_PARAMETERS:
            return "Cooldown parameters";
        case AUTHORIZATION_TIME_PARAMETERS:
            return "Time parameters";
        case AUTHORIZATION_CREATE_PLT:
            return "Create PLT";
        default:
            THROW(ERROR_INVALID_STATE);
    }
}

typedef enum { AUTHS_V0, AUTHS_V1, AUTHS_V2 } authsVersion_e;
static authsVersion_e authsVersion;

/**
 * Method to be called when the user validates the threshold in the UI. If there are
 * additional authorization types to process, then ask for additional data, otherwise
 * continue to the signing flow as this marks the end of the transaction.
 */
void processThreshold(void) {
    ctx->authorizationType += 1;

    uint8_t endMarker;
    switch (authsVersion) {
        case AUTHS_V0:
            endMarker = AUTHORIZATION_END_V0;
            break;
        case AUTHS_V1:
            endMarker = AUTHORIZATION_END_V1;
            break;
        case AUTHS_V2:
            endMarker = AUTHORIZATION_END_V2;
            break;
    }

    if (ctx->authorizationType == endMarker) {
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
void processKeyIndices(void) {
    if (ctx->accessStructureSize == 0) {
        if (ctx->processedCount != 0) {
            // We have been given more indices than promised
            THROW(ERROR_INVALID_TRANSACTION);
        }
        // The current access structure has been fully processed, continue to the threshold
        // for the current access structure.
        ctx->state = TX_UPDATE_AUTHORIZATIONS_ACCESS_STRUCTURE_THRESHOLD;
        sendSuccessNoIdle();
    } else if (ctx->processedCount == 0) {
        // The current batch was processed, but there are more to be processed. Ask for more data.
        sendSuccessNoIdle();
    } else {
        uint16_t keyIndex = U2BE(ctx->buffer, ctx->bufferPointer);
        bin2dec(ctx->displayKeyIndex, sizeof(ctx->displayKeyIndex), keyIndex);
        memmove(ctx->title, getAuthorizationName(ctx->authorizationType), 29);
        ctx->bufferPointer += 2;
        ctx->accessStructureSize -= 1;
        ctx->processedCount -= 1;
        ux_flow_init(0, ux_update_authorizations, NULL);
    }
}

/**
 * Sets the display text for the update type.
 *
 * @param updateType The type of update:
 *  - UPDATE_TYPE_UPDATE_ROOT_KEYS (10): Update using root keys
 *  - UPDATE_TYPE_UPDATE_LEVEL1_KEYS (11): Update using level 1 keys
 * @throws ERROR_INVALID_TRANSACTION if the parameters don't match expected values
 */
void setTypeText(uint8_t updateType) {
    switch (updateType) {
        case UPDATE_TYPE_UPDATE_ROOT_KEYS:
            memmove(ctx->type, "Level 2 w. root keys", 21);
            break;
        case UPDATE_TYPE_UPDATE_LEVEL1_KEYS:
            memmove(ctx->type, "Level 2 w. level 1 keys", 24);
            break;
        default:
            THROW(ERROR_INVALID_TRANSACTION);
    }
}

#define P1_INITIAL \
    0x00  // Contains key derivation path, update header and update kind, update key type and count of incoming public
          // update keys.
#define P1_PUBLIC_KEY                 0x01  // Contains one public-key per command.
#define P1_ACCESS_STRUCTURE_INITIAL   0x02  // Contains the number of public-key indices for the current access.
#define P1_ACCESS_STRUCTURE           0x03  // Contains the public-key indices for the current access structure.
#define P1_ACCESS_STRUCTURE_THRESHOLD 0x04  // Contains the threshold for the current access structure.

#define P2_V0 AUTHS_V0
#define P2_V1 AUTHS_V1
#define P2_V2 AUTHS_V2

void handleSignUpdateAuthorizations(
    uint8_t *cdata,
    uint8_t p1,
    uint8_t p2,
    uint8_t updateType,
    uint8_t dataLength,
    volatile unsigned int *flags,
    bool isInitialCall) {
    switch (p2) {
        case P2_V0:
        case P2_V1:
        case P2_V2:
            break;
        default:
            THROW(ERROR_INVALID_PARAM);
    }

    if (isInitialCall) {
        ctx->state = TX_UPDATE_AUTHORIZATIONS_INITIAL;
    }

    if (p1 == P1_INITIAL && ctx->state == TX_UPDATE_AUTHORIZATIONS_INITIAL) {
        cdata += parseKeyDerivationPath(cdata);
        cx_sha256_init(&tx_state->hash);
        cdata += hashUpdateHeaderAndType(cdata, updateType);

        uint8_t keyUpdateType = cdata[0];
        updateHash((cx_hash_t *) &tx_state->hash, cdata, 1);
        cdata += 1;

        if (updateType == UPDATE_TYPE_UPDATE_ROOT_KEYS) {
            switch (keyUpdateType) {
                case ROOT_UPDATE_LEVEL_2_V0:
                    authsVersion = AUTHS_V0;
                    break;
                case ROOT_UPDATE_LEVEL_2_V1:
                    authsVersion = AUTHS_V1;
                    break;
                case ROOT_UPDATE_LEVEL_2_V2:
                    authsVersion = AUTHS_V2;
                    break;
                default:
                    THROW(ERROR_INVALID_TRANSACTION);
            }
        } else if (updateType == UPDATE_TYPE_UPDATE_LEVEL1_KEYS) {
            switch (keyUpdateType) {
                case LEVEL1_UPDATE_LEVEL_2_V0:
                    authsVersion = AUTHS_V0;
                    break;
                case LEVEL1_UPDATE_LEVEL_2_V1:
                    authsVersion = AUTHS_V1;
                    break;
                case LEVEL1_UPDATE_LEVEL_2_V2:
                    authsVersion = AUTHS_V2;
                    break;
                default:
                    THROW(ERROR_INVALID_TRANSACTION);
            }
        } else {
            THROW(ERROR_INVALID_TRANSACTION);
        }

        // Validate p2 against auths type (these should match)
        if (authsVersion != p2) {
            THROW(ERROR_INVALID_PARAM);
        }

        ctx->publicKeyListLength = U2BE(cdata, 0);
        updateHash((cx_hash_t *) &tx_state->hash, cdata, 2);

        setTypeText(updateType);

        ctx->state = TX_UPDATE_AUTHORIZATIONS_PUBLIC_KEY;
        ctx->authorizationType = 0;

        ux_flow_init(0, ux_sign_update_authorizations_review, NULL);
        *flags |= IO_ASYNCH_REPLY;
    } else if (
        p1 == P1_PUBLIC_KEY && ctx->state == TX_UPDATE_AUTHORIZATIONS_PUBLIC_KEY && ctx->publicKeyListLength > 0) {
        // Hash the schemeId
        updateHash((cx_hash_t *) &tx_state->hash, cdata, 1);
        cdata += 1;

        uint8_t publicKeyInput[32];
        memmove(publicKeyInput, cdata, 32);
        toPaginatedHex(publicKeyInput, 32, ctx->publicKey, sizeof(ctx->publicKey));
        updateHash((cx_hash_t *) &tx_state->hash, publicKeyInput, 32);

        ctx->publicKeyListLength -= 1;
        if (ctx->publicKeyListLength == 0) {
            ctx->state = TX_UPDATE_AUTHORIZATIONS_ACCESS_STRUCTURE_SIZE;
        }
        ux_flow_init(0, ux_update_authorizations_public_key, NULL);
        *flags |= IO_ASYNCH_REPLY;
    } else if (p1 == P1_ACCESS_STRUCTURE_INITIAL && ctx->state == TX_UPDATE_AUTHORIZATIONS_ACCESS_STRUCTURE_SIZE) {
        updateHash((cx_hash_t *) &tx_state->hash, cdata, 2);
        ctx->accessStructureSize = U2BE(cdata, 0);
        ctx->state = TX_UPDATE_AUTHORIZATIONS_ACCESS_STRUCTURE_INDEX;
        sendSuccessNoIdle();
    } else if (p1 == P1_ACCESS_STRUCTURE && ctx->state == TX_UPDATE_AUTHORIZATIONS_ACCESS_STRUCTURE_INDEX) {
        ctx->bufferPointer = 0;
        if (dataLength % 2 == 1) {
            THROW(ERROR_INVALID_TRANSACTION);
        }
        ctx->processedCount = dataLength / 2;
        updateHash((cx_hash_t *) &tx_state->hash, cdata, dataLength);
        memmove(ctx->buffer, cdata, dataLength);
        processKeyIndices();
        *flags |= IO_ASYNCH_REPLY;

    } else if (
        p1 == P1_ACCESS_STRUCTURE_THRESHOLD && ctx->state == TX_UPDATE_AUTHORIZATIONS_ACCESS_STRUCTURE_THRESHOLD) {
        uint16_t threshold = U2BE(cdata, 0);
        updateHash((cx_hash_t *) &tx_state->hash, cdata, 2);
        bin2dec(ctx->displayKeyIndex, sizeof(ctx->displayKeyIndex), threshold);
        memmove(ctx->title, "Threshold", 10);

        ux_flow_init(0, ux_update_authorizations_threshold, NULL);
        *flags |= IO_ASYNCH_REPLY;
    } else {
        THROW(ERROR_INVALID_STATE);
    }
}
