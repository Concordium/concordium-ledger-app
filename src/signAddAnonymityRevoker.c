#include <os.h>
#include "util.h"
#include "sign.h"
#include "responseCodes.h"

static signAddAnonymityRevokerContext_t *ctx = &global.signAddAnonymityRevokerContext;
static tx_state_t *tx_state = &global_tx_state;

UX_STEP_CB(
           ux_sign_add_anonymity_revoker_arIdentity,
           bnnn_paging,
           sendSuccessNoIdle(),
           {
             "arIdentity",
               (char *) global.signAddAnonymityRevokerContext.arIdentity
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
          (char *) global.signAddAnonymityRevokerContext.publicKey
    });
UX_FLOW(ux_sign_add_anonymity_revoker_finish,
        &ux_sign_add_anonymity_revoker_public_key,
        &ux_sign_flow_shared_sign,
        &ux_sign_flow_shared_decline
);

#define P1_INITIAL              0x00
#define P1_DESCRIPTION_LENGTH          0x01        // Used for both the message text and the specification URL.
#define P1_DESCRIPTION                 0x02        // Used for both the message text and the specification URL.
#define P1_PUBLIC_KEY       0x03

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
        cdata += 4;

        // Read the arIdentity.
        uint32_t arIdentity = U4BE(cdata, 0);
        cx_hash((cx_hash_t *) &tx_state->hash, 0, cdata, 4, NULL, 0);
        numberToText(ctx->arIdentity, arIdentity);
        cdata += 4;

        ctx->state = TX_ADD_ANONYMITY_REVOKER_DESCRIPTION_LENGTH;

        ux_flow_init(0, ux_sign_add_anonymity_revoker_start, NULL);
        *flags |= IO_ASYNCH_REPLY;

    } else if (p1 == P1_DESCRIPTION_LENGTH && ctx->state == TX_ADD_ANONYMITY_REVOKER_DESCRIPTION_LENGTH) {
        // Read description length
        ctx->descriptionLength = U4BE(cdata, 0);
        cdata += 4;

        ctx->state = TX_ADD_ANONYMITY_REVOKER_DESCRIPTION;
        sendSuccessNoIdle();
    } else if (p1 == P1_DESCRIPTION && ctx->state == TX_ADD_ANONYMITY_REVOKER_DESCRIPTION) {
      cx_hash((cx_hash_t *) &tx_state->hash, 0, cdata, dataLength, NULL, 0);
      ctx->descriptionLength -= dataLength;

      if (ctx->descriptionLength == 0) {
        // We have received all bytes, continue to signing flow.
        ctx->state = TX_ADD_ANONYMITY_REVOKER_PUBLIC_KEY;
        sendSuccessNoIdle();
      } else if (ctx->descriptionLength < 0) {
        // We received more bytes than expected.
        THROW(ERROR_INVALID_STATE);
      } else {
        // There are more bytes to be received. Ask the computer for more.
        sendSuccessNoIdle();
      }
    } else if (p1 == P1_PUBLIC_KEY && ctx->state == TX_ADD_ANONYMITY_REVOKER_PUBLIC_KEY) {
        uint8_t publicKey[96];
        memmove(publicKey, cdata, 96);
        cx_hash((cx_hash_t *) &tx_state->hash, 0, publicKey, 96, NULL, 0);
        toHex(publicKey, 96, ctx->publicKey);
        cdata += 96;

        ux_flow_init(0, ux_sign_add_anonymity_revoker_finish, NULL);
        *flags |= IO_ASYNCH_REPLY;
    } else {
        THROW(ERROR_INVALID_STATE);
    }
}
