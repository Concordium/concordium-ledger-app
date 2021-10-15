#include <os.h>
#include "util.h"
#include "sign.h"
#include "responseCodes.h"
#include "descriptionView.h"

static signAddAnonymityRevokerContext_t *ctx = &global.withDescription.signAddAnonymityRevokerContext;
static descriptionContext_t *desc_ctx = &global.withDescription.descriptionContext;
static tx_state_t *tx_state = &global_tx_state;

UX_STEP_CB(
    ux_sign_add_anonymity_revoker_arIdentity,
    bn,
    sendSuccessNoIdle(),
    {
        "AR Identity",
        (char *) global.withDescription.signAddAnonymityRevokerContext.arIdentity
    });
UX_FLOW(ux_sign_add_anonymity_revoker_start,
    &ux_sign_flow_shared_review,
    &ux_sign_add_anonymity_revoker_arIdentity
);

UX_STEP_NOCB(
    ux_sign_add_anonymity_revoker_public_key,
    bnnn_paging,
    {
        "Public key",
        (char *) global.withDescription.signAddAnonymityRevokerContext.publicKey
    });
UX_FLOW(ux_sign_add_anonymity_revoker_finish,
    &ux_sign_add_anonymity_revoker_public_key,
    &ux_sign_flow_shared_sign,
    &ux_sign_flow_shared_decline
);

#define P1_INITIAL                     0x00
#define P1_DESCRIPTION_LENGTH          0x01        // Used for the name, url, description.
#define P1_DESCRIPTION                 0x02        // Used for the name, url, description.
#define P1_PUBLIC_KEY                  0x03

void checkIfDescriptionPartIsDoneAnonymityRevoker(void) {
    if (desc_ctx->textLength==0) {
        // If we have received all of the current part of the description, update the state.
        switch (desc_ctx->descriptionState) {
            case DESC_DESCRIPTION:
                ctx->state = TX_ADD_ANONYMITY_REVOKER_PUBLIC_KEY;
                break;
            default:
                ctx->state = TX_ADD_ANONYMITY_REVOKER_DESCRIPTION_LENGTH;
                break;
        }
    }
}

void handleSignAddAnonymityRevoker(uint8_t *cdata, uint8_t p1, uint8_t dataLength, volatile unsigned int *flags, bool isInitialCall) {
    if (isInitialCall) {
        ctx->state = TX_ADD_ANONYMITY_REVOKER_INITIAL;
    }

    if (p1 == P1_INITIAL && ctx->state == TX_ADD_ANONYMITY_REVOKER_INITIAL) {
        int bytesRead = parseKeyDerivationPath(cdata);
        cdata += bytesRead;

        cx_sha256_init(&tx_state->hash);
        cdata += hashUpdateHeaderAndType(cdata, UPDATE_TYPE_ADD_ANONYMITY_REVOKER);

        // Read the payload length.
        cx_hash((cx_hash_t *) &tx_state->hash, 0, cdata, 4, NULL, 0);
        ctx->payloadLength = U4BE(cdata, 0);
        cdata += 4;

        // Read the arIdentity.
        uint32_t arIdentity = U4BE(cdata, 0);
        cx_hash((cx_hash_t *) &tx_state->hash, 0, cdata, 4, NULL, 0);
        numberToText(ctx->arIdentity, arIdentity);

        ctx->state = TX_ADD_ANONYMITY_REVOKER_DESCRIPTION_LENGTH;
        desc_ctx->descriptionState = DESC_NAME;

        ux_flow_init(0, ux_sign_add_anonymity_revoker_start, NULL);
        *flags |= IO_ASYNCH_REPLY;

    } else if (p1 == P1_DESCRIPTION_LENGTH && ctx->state == TX_ADD_ANONYMITY_REVOKER_DESCRIPTION_LENGTH) {
        // Read current part of description length
        desc_ctx->textLength = U4BE(cdata, 0);
        cx_hash((cx_hash_t *) &tx_state->hash, 0, cdata, 4, NULL, 0);
        ctx->payloadLength -= 4;

        ctx->state = TX_ADD_ANONYMITY_REVOKER_DESCRIPTION;
        
        // We do this to handle if this part of the description is empty
        checkIfDescriptionPartIsDoneAnonymityRevoker();
        handleDescriptionPart();
    } else if (p1 == P1_DESCRIPTION && ctx->state == TX_ADD_ANONYMITY_REVOKER_DESCRIPTION) {
        if (desc_ctx->textLength < dataLength) {
            // We received more bytes than expected.
            THROW(ERROR_INVALID_STATE);
        }

        cx_hash((cx_hash_t *) &tx_state->hash, 0, cdata, dataLength, NULL, 0);

        memmove(desc_ctx->text, cdata, dataLength);

        ctx->payloadLength -= dataLength;
        desc_ctx->textLength -= dataLength;

        if (dataLength < 255) {
            memmove(desc_ctx->text + dataLength, "\0", 1);
        }

        checkIfDescriptionPartIsDoneAnonymityRevoker();
        displayDescriptionPart(flags);
    } else if (p1 == P1_PUBLIC_KEY && ctx->state == TX_ADD_ANONYMITY_REVOKER_PUBLIC_KEY) {
        uint8_t publicKey[96];
        memmove(publicKey, cdata, 96);
        cx_hash((cx_hash_t *) &tx_state->hash, 0, publicKey, 96, NULL, 0);
        toPaginatedHex(publicKey, 96, ctx->publicKey);

        ux_flow_init(0, ux_sign_add_anonymity_revoker_finish, NULL);
        *flags |= IO_ASYNCH_REPLY;
    } else {
        THROW(ERROR_INVALID_STATE);
    }
}
