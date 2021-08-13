#include <os.h>
#include "util.h"
#include "sign.h"
#include "responseCodes.h"

static signAddIdentityProviderContext_t *ctx = &global.withDescription.signAddIdentityProviderContext;
static descriptionContext_t *desc_ctx = &global.withDescription.descriptionContext;
static tx_state_t *tx_state = &global_tx_state;

UX_STEP_CB(
    ux_sign_add_identity_provider_ipIdentity,
    bnnn_paging,
    sendSuccessNoIdle(),
    {
        "Identity provider",
            (char *) global.withDescription.signAddIdentityProviderContext.ipIdentity
            });
UX_FLOW(ux_sign_add_identity_provider_start,
        &ux_sign_flow_shared_review,
        &ux_sign_add_identity_provider_ipIdentity
    );

UX_STEP_CB(
    ux_sign_add_identity_provider_verify_key_hash,
    bnnn_paging,
    sendSuccessNoIdle(),
    {
        "Verify Key Hash",
            (char *) global.withDescription.signAddIdentityProviderContext.verifyKeyHash
            });
UX_FLOW(ux_sign_add_identity_provider_verify_key,
        &ux_sign_add_identity_provider_verify_key_hash
    );

UX_STEP_NOCB(
    ux_sign_add_identity_provider_cdi_key,
    bnnn_paging,
    {
        "CDI Verify key",
            (char *) global.withDescription.signAddIdentityProviderContext.cdiVerifyKey
            });
UX_FLOW(ux_sign_add_identity_provider_finish,
        &ux_sign_add_identity_provider_cdi_key,
        &ux_sign_flow_shared_sign,
        &ux_sign_flow_shared_decline
    );

#define P1_INITIAL              0x00
#define P1_DESCRIPTION_LENGTH          0x01        // Used for the name, url, description.
#define P1_DESCRIPTION                 0x02        // Used for the name, url, description.
#define P1_VERIFY_KEY       0x03
#define P1_CDI_VERIFY_KEY       0x04

#define CDI_VERIFY_KEY_LENGTH 32

void handleSignAddIdentityProvider(uint8_t *cdata, uint8_t p1, uint8_t dataLength, volatile unsigned int *flags, bool isInitialCall) {
    if (isInitialCall) {
        ctx->state = TX_ADD_IDENTITY_PROVIDER_INITIAL;
    }

    if (p1 == P1_INITIAL && ctx->state == TX_ADD_IDENTITY_PROVIDER_INITIAL) {
        int bytesRead = parseKeyDerivationPath(cdata);
        cdata += bytesRead;

        cx_sha256_init(&tx_state->hash);
        cx_sha256_init(&ctx->hash);
        cdata += hashUpdateHeaderAndType(cdata, UPDATE_TYPE_ADD_IDENTITY_PROVIDER);

        // Read the IpInfo length.
        cx_hash((cx_hash_t *) &tx_state->hash, 0, cdata, 4, NULL, 0);
        ctx->payloadLength = U4BE(cdata, 0);
        cdata += 4;

        // Read the ipIdentity.
        uint32_t ipIdentity = U4BE(cdata, 0);
        cx_hash((cx_hash_t *) &tx_state->hash, 0, cdata, 4, NULL, 0);
        bin2dec(ctx->ipIdentity, ipIdentity);
        ctx->payloadLength -= 4;
        cdata += 4;

        ctx->state = TX_ADD_IDENTITY_PROVIDER_DESCRIPTION_LENGTH;
        desc_ctx->descriptionState = DESC_NAME;

        ux_flow_init(0, ux_sign_add_identity_provider_start, NULL);
        *flags |= IO_ASYNCH_REPLY;

    } else if (p1 == P1_DESCRIPTION_LENGTH && ctx->state == TX_ADD_IDENTITY_PROVIDER_DESCRIPTION_LENGTH) {
        // Read current part of description length
        desc_ctx->textLength = U4BE(cdata, 0);
        cx_hash((cx_hash_t *) &tx_state->hash, 0, cdata, 4, NULL, 0);
        ctx->payloadLength -= 4;
        cdata += 4;

        ctx->state = TX_ADD_IDENTITY_PROVIDER_DESCRIPTION;
        sendSuccessNoIdle();
    } else if (p1 == P1_DESCRIPTION && ctx->state == TX_ADD_IDENTITY_PROVIDER_DESCRIPTION) {
        cx_hash((cx_hash_t *) &tx_state->hash, 0, cdata, dataLength, NULL, 0);

        memmove(desc_ctx->text, cdata, dataLength);

        ctx->payloadLength -= dataLength;
        desc_ctx->textLength -= dataLength;

        if (dataLength < 255) {
            memmove(desc_ctx->text + dataLength, "\0", 1);
        }

        if (desc_ctx->textLength < 0) {
            // We received more bytes than expected.
            THROW(ERROR_INVALID_STATE);
            return;
        }

        switch (desc_ctx->descriptionState) {
        case DESC_NAME:
            if (desc_ctx->textLength==0) {
                ctx->state = TX_ADD_IDENTITY_PROVIDER_DESCRIPTION_LENGTH;
            }
            ux_flow_init(0, ux_sign_description_name, NULL);
            *flags |= IO_ASYNCH_REPLY;
            break;
        case DESC_URL:
            if (desc_ctx->textLength==0) {
                ctx->state = TX_ADD_IDENTITY_PROVIDER_DESCRIPTION_LENGTH;
            }
            ux_flow_init(0, ux_sign_description_url, NULL);
            *flags |= IO_ASYNCH_REPLY;
            break;
        case DESC_DESCRIPTION:
            if (desc_ctx->textLength==0) {
                ctx->state = TX_ADD_IDENTITY_PROVIDER_VERIFY_KEY;
                ctx->verifyKeyLength = ctx->payloadLength - CDI_VERIFY_KEY_LENGTH;
            }
            ux_flow_init(0, ux_sign_description_description, NULL);
            *flags |= IO_ASYNCH_REPLY;
            break;
        default:
            THROW(ERROR_INVALID_STATE);
            break;
        }
    } else if (p1 == P1_VERIFY_KEY && ctx->state == TX_ADD_IDENTITY_PROVIDER_VERIFY_KEY) {
        cx_hash((cx_hash_t *) &tx_state->hash, 0, cdata, dataLength, NULL, 0);
        cx_hash((cx_hash_t *) &ctx->hash, 0, cdata, dataLength, NULL, 0);
        ctx->verifyKeyLength -= dataLength;
        ctx->payloadLength -= dataLength;

        if (ctx->verifyKeyLength == 0) {
            // We have received all bytes of the verifyKey.
            uint8_t hashBytes[32];
            cx_hash((cx_hash_t *) &ctx->hash, CX_LAST, NULL, 0, hashBytes, 32);
            toHex(hashBytes, 32, ctx->verifyKeyHash);

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
        cx_hash((cx_hash_t *) &tx_state->hash, 0, cdata, CDI_VERIFY_KEY_LENGTH, NULL, 0);
        toHex(cdata, CDI_VERIFY_KEY_LENGTH, ctx->cdiVerifyKey);
        ctx->payloadLength -= CDI_VERIFY_KEY_LENGTH;
        cdata += CDI_VERIFY_KEY_LENGTH;

        if (dataLength != CDI_VERIFY_KEY_LENGTH || ctx->payloadLength != 0) {
            // We received more bytes than expected.
            THROW(ERROR_INVALID_STATE);
        } else {
            ux_flow_init(0, ux_sign_add_identity_provider_finish, NULL);
            *flags |= IO_ASYNCH_REPLY;
        }
    } else {
        THROW(ERROR_INVALID_STATE);
    }
}
