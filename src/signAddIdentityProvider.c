#include <os.h>
#include "util.h"
#include "sign.h"
#include "responseCodes.h"

static signAddIdentityProviderContext_t *ctx = &global.signAddIdentityProviderContext;
static tx_state_t *tx_state = &global_tx_state;

UX_STEP_CB(
           ux_sign_add_identity_provider_ipIdentity,
           bnnn_paging,
           sendSuccessNoIdle(),
           {
             "ipIdentity",
               (char *) global.signAddIdentityProviderContext.ipIdentity
               });
UX_FLOW(ux_sign_add_identity_provider_start,
        &ux_sign_flow_shared_review,
        &ux_sign_add_identity_provider_ipIdentity
        );

UX_STEP_NOCB(
    ux_sign_add_identity_provider_cdi_key,
    bnnn_paging,
    {
        "cdi Verify key",
          (char *) global.signAddIdentityProviderContext.cdiVerifyKey
    });
UX_FLOW(ux_sign_add_identity_provider_finish,
        &ux_sign_add_identity_provider_cdi_key,
        &ux_sign_flow_shared_sign,
        &ux_sign_flow_shared_decline
);

#define P1_INITIAL              0x00
#define P1_DESCRIPTION_LENGTH          0x01        // Used for both the message text and the specification URL.
#define P1_DESCRIPTION                 0x02        // Used for both the message text and the specification URL.
#define P1_VERIFY_KEY_LENGTH   0x03
#define P1_VERIFY_KEY       0x04
#define P1_CDI_VERIFY_KEY       0x05

void handleSignAddIdentityProvider(uint8_t *cdata, uint8_t p1, uint8_t dataLength, volatile unsigned int *flags, bool isInitialCall) {
    if (isInitialCall) {
      ctx->state = TX_ADD_IDENTITY_PROVIDER_INITIAL;
    }

    if (p1 == P1_INITIAL && ctx->state == TX_ADD_IDENTITY_PROVIDER_INITIAL) {
        int bytesRead = parseKeyDerivationPath(cdata);
        cdata += bytesRead;

        cx_sha256_init(&tx_state->hash);
        cdata += hashUpdateHeaderAndType(cdata, UPDATE_TYPE_ADD_IDENTITY_PROVIDER);

        // Read the payload length.
        cx_hash((cx_hash_t *) &tx_state->hash, 0, cdata, 4, NULL, 0);
        cdata += 4;

        // Read the ipIdentity.
        uint32_t ipIdentity = U4BE(cdata, 0);
        cx_hash((cx_hash_t *) &tx_state->hash, 0, cdata, 4, NULL, 0);
        numberToText(ctx->ipIdentity, ipIdentity);
        cdata += 4;

        ctx->state = TX_ADD_IDENTITY_PROVIDER_DESCRIPTION_LENGTH;

        ux_flow_init(0, ux_sign_add_identity_provider_start, NULL);
        *flags |= IO_ASYNCH_REPLY;

    } else if (p1 == P1_DESCRIPTION_LENGTH && ctx->state == TX_ADD_IDENTITY_PROVIDER_DESCRIPTION_LENGTH) {
        // Read description length
        ctx->descriptionLength = U4BE(cdata, 0);
        cdata += 4;

        ctx->state = TX_ADD_IDENTITY_PROVIDER_DESCRIPTION;
        sendSuccessNoIdle();
    } else if (p1 == P1_DESCRIPTION && ctx->state == TX_ADD_IDENTITY_PROVIDER_DESCRIPTION) {
      cx_hash((cx_hash_t *) &tx_state->hash, 0, cdata, dataLength, NULL, 0);
      ctx->descriptionLength -= dataLength;

      if (ctx->descriptionLength == 0) {
        // We have received all bytes, continue to signing flow.
        ctx->state = TX_ADD_IDENTITY_PROVIDER_VERIFY_KEY_LENGTH;
        sendSuccessNoIdle();
      } else if (ctx->descriptionLength < 0) {
        // We received more bytes than expected.
        THROW(ERROR_INVALID_STATE);
      } else {
        // There are more bytes to be received. Ask the computer for more.
        sendSuccessNoIdle();
      }
    } else if (p1 == P1_VERIFY_KEY_LENGTH && ctx->state == TX_ADD_IDENTITY_PROVIDER_VERIFY_KEY_LENGTH) {
        // Read description length
        ctx->verifyKeyLength = U4BE(cdata, 0);
        cdata += 4;

        ctx->state = TX_ADD_IDENTITY_PROVIDER_VERIFY_KEY;
        sendSuccessNoIdle();
    } else if (p1 == P1_VERIFY_KEY && ctx->state == TX_ADD_IDENTITY_PROVIDER_VERIFY_KEY) {
      cx_hash((cx_hash_t *) &tx_state->hash, 0, cdata, dataLength, NULL, 0);
      ctx->verifyKeyLength -= dataLength;

      if (ctx->verifyKeyLength == 0) {
        // We have received all bytes, continue to signing flow.
        ctx->state = TX_ADD_IDENTITY_PROVIDER_CDI_VERIFY_KEY;
        sendSuccessNoIdle();
      } else if (ctx->verifyKeyLength < 0) {
        // We received more bytes than expected.
        THROW(ERROR_INVALID_STATE);
      } else {
        // There are more bytes to be received. Ask the computer for more.
        sendSuccessNoIdle();
      }
    } else if (p1 == P1_CDI_VERIFY_KEY && ctx->state == TX_ADD_IDENTITY_PROVIDER_CDI_VERIFY_KEY) {
        uint8_t publicKey[32];
        memmove(publicKey, cdata, 32);
        cx_hash((cx_hash_t *) &tx_state->hash, 0, publicKey, 32, NULL, 0);
        toHex(publicKey, 32, ctx->cdiVerifyKey);
        cdata += 32;


        ux_flow_init(0, ux_sign_add_identity_provider_finish, NULL);
        *flags |= IO_ASYNCH_REPLY;
    } else {
        THROW(ERROR_INVALID_STATE);
    }
}
