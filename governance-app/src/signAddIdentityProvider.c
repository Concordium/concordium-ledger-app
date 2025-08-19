#include <os.h>

#include "responseCodes.h"
#include "sign.h"
#include "util.h"

static signAddIdentityProviderContext_t *ctx = &global.withDescription.signAddIdentityProviderContext;
static descriptionContext_t *desc_ctx = &global.withDescription.descriptionContext;
static tx_state_t *tx_state = &global_tx_state;

UX_STEP_NOCB(
    ux_sign_add_identity_provider_type_step,
    bn,
    {"Update type", (char *) global.withDescription.signAddIdentityProviderContext.updateTypeText});
UX_STEP_CB(
    ux_sign_add_identity_provider_ipIdentity,
    bn,
    sendSuccessNoIdle(),
    {"Identity provider", (char *) global.withDescription.signAddIdentityProviderContext.ipIdentity});
UX_FLOW(
    ux_sign_add_identity_provider_start,
    &ux_sign_flow_shared_review,
    &ux_sign_add_identity_provider_type_step,
    &ux_sign_add_identity_provider_ipIdentity);

UX_STEP_CB(
    ux_sign_add_identity_provider_verify_key_hash,
    bnnn_paging,
    sendSuccessNoIdle(),
    {"Verify Key Hash", (char *) global.withDescription.signAddIdentityProviderContext.verifyKeyHash});
UX_FLOW(ux_sign_add_identity_provider_verify_key, &ux_sign_add_identity_provider_verify_key_hash);

UX_STEP_NOCB(
    ux_sign_add_identity_provider_cdi_key,
    bnnn_paging,
    {"CDI Verify key", (char *) global.withDescription.signAddIdentityProviderContext.cdiVerifyKey});
UX_FLOW(
    ux_sign_add_identity_provider_finish,
    &ux_sign_add_identity_provider_cdi_key,
    &ux_sign_flow_shared_sign,
    &ux_sign_flow_shared_decline);

#define P1_INITIAL            0x00
#define P1_DESCRIPTION_LENGTH 0x01  // Used for the name, url, description.
#define P1_DESCRIPTION        0x02  // Used for the name, url, description.
#define P1_VERIFY_KEY         0x03
#define P1_CDI_VERIFY_KEY     0x04

#define CDI_VERIFY_KEY_LENGTH 32

void checkIfDescriptionPartIsDoneIdentityProvider() {
    if (desc_ctx->textLength == 0) {
        // If we have received all of the current part of the description, update the state.
        switch (desc_ctx->descriptionState) {
            case DESC_DESCRIPTION:
                ctx->state = TX_ADD_IDENTITY_PROVIDER_VERIFY_KEY;
                ctx->verifyKeyLength = ctx->payloadLength - CDI_VERIFY_KEY_LENGTH;
                break;
            default:
                ctx->state = TX_ADD_IDENTITY_PROVIDER_DESCRIPTION_LENGTH;
                break;
        }
    }
}

void handleSignAddIdentityProvider(
    uint8_t *cdata,
    uint8_t p1,
    uint8_t dataLength,
    volatile unsigned int *flags,
    bool isInitialCall) {
    if (isInitialCall) {
        ctx->state = TX_ADD_IDENTITY_PROVIDER_INITIAL;
    }

    if (p1 == P1_INITIAL && ctx->state == TX_ADD_IDENTITY_PROVIDER_INITIAL) {
        cdata += parseKeyDerivationPath(cdata);

        cx_sha256_init(&tx_state->hash);
        cx_sha256_init(&ctx->hash);
        cdata += hashUpdateHeaderAndType(cdata, UPDATE_TYPE_ADD_IDENTITY_PROVIDER);

        // Set update type text for display
        strncpy(ctx->updateTypeText, getUpdateTypeText(UPDATE_TYPE_ADD_IDENTITY_PROVIDER), sizeof(ctx->updateTypeText));
        ctx->updateTypeText[sizeof(ctx->updateTypeText) - 1] = '\0';

        // Read the IpInfo length.
        updateHash((cx_hash_t *) &tx_state->hash, cdata, 4);
        ctx->payloadLength = U4BE(cdata, 0);
        cdata += 4;

        // Read the ipIdentity.
        uint32_t ipIdentity = U4BE(cdata, 0);
        updateHash((cx_hash_t *) &tx_state->hash, cdata, 4);
        bin2dec(ctx->ipIdentity, sizeof(ctx->ipIdentity), ipIdentity);
        ctx->payloadLength -= 4;

        ctx->state = TX_ADD_IDENTITY_PROVIDER_DESCRIPTION_LENGTH;
        desc_ctx->descriptionState = DESC_NAME;

        ux_flow_init(0, ux_sign_add_identity_provider_start, NULL);
        *flags |= IO_ASYNCH_REPLY;

    } else if (p1 == P1_DESCRIPTION_LENGTH && ctx->state == TX_ADD_IDENTITY_PROVIDER_DESCRIPTION_LENGTH) {
        // Read current part of description length
        desc_ctx->textLength = U4BE(cdata, 0);
        updateHash((cx_hash_t *) &tx_state->hash, cdata, 4);
        ctx->payloadLength -= 4;

        ctx->state = TX_ADD_IDENTITY_PROVIDER_DESCRIPTION;

        checkIfDescriptionPartIsDoneIdentityProvider();
        handleDescriptionPart();
    } else if (p1 == P1_DESCRIPTION && ctx->state == TX_ADD_IDENTITY_PROVIDER_DESCRIPTION) {
        if (desc_ctx->textLength < dataLength) {
            // We received more bytes than expected.
            THROW(ERROR_INVALID_STATE);
            return;
        }

        updateHash((cx_hash_t *) &tx_state->hash, cdata, dataLength);

        memmove(desc_ctx->text, cdata, dataLength);

        ctx->payloadLength -= dataLength;
        desc_ctx->textLength -= dataLength;

        if (dataLength < 255) {
            memmove(desc_ctx->text + dataLength, "\0", 1);
        }

        // We do this to handle if this part of the description is empty
        checkIfDescriptionPartIsDoneIdentityProvider();
        displayDescriptionPart(flags);
    } else if (p1 == P1_VERIFY_KEY && ctx->state == TX_ADD_IDENTITY_PROVIDER_VERIFY_KEY) {
        updateHash((cx_hash_t *) &tx_state->hash, cdata, dataLength);
        updateHash((cx_hash_t *) &ctx->hash, cdata, dataLength);
        ctx->verifyKeyLength -= dataLength;
        ctx->payloadLength -= dataLength;

        if (ctx->verifyKeyLength == 0) {
            // We have received all bytes of the verifyKey.
            uint8_t hashBytes[32];
            hash((cx_hash_t *) &ctx->hash, CX_LAST, NULL, 0, hashBytes, 32);
            toPaginatedHex(hashBytes, 32, ctx->verifyKeyHash, sizeof(ctx->verifyKeyHash));

            ctx->state = TX_ADD_IDENTITY_PROVIDER_CDI_VERIFY_KEY;
            ux_flow_init(0, ux_sign_add_identity_provider_verify_key, NULL);
            *flags |= IO_ASYNCH_REPLY;
        } else if (ctx->verifyKeyLength < 0) {
            // We received more bytes than expected.
            THROW(ERROR_INVALID_STATE);
        } else {
            // There are more bytes to be received. Ask the computer for more.
            sendSuccessNoIdle();
        }
    } else if (p1 == P1_CDI_VERIFY_KEY && ctx->state == TX_ADD_IDENTITY_PROVIDER_CDI_VERIFY_KEY) {
        updateHash((cx_hash_t *) &tx_state->hash, cdata, CDI_VERIFY_KEY_LENGTH);
        toPaginatedHex(cdata, CDI_VERIFY_KEY_LENGTH, ctx->cdiVerifyKey, sizeof(ctx->cdiVerifyKey));
        ctx->payloadLength -= CDI_VERIFY_KEY_LENGTH;

        if (dataLength != CDI_VERIFY_KEY_LENGTH || ctx->payloadLength != 0) {
            // the CDI_VERIFY_KEY or the entire payload did not have the expected length.
            THROW(ERROR_INVALID_STATE);
        } else {
            ux_flow_init(0, ux_sign_add_identity_provider_finish, NULL);
            *flags |= IO_ASYNCH_REPLY;
        }
    } else {
        THROW(ERROR_INVALID_STATE);
    }
}
