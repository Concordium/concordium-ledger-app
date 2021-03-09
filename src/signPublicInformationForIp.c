#include <os.h>
#include <os_io_seproxyhal.h>
#include "cx.h"
#include <stdint.h>
#include "util.h"
#include <stdio.h>
#include "sign.h"

static signPublicInformationForIp_t *ctx = &global.signPublicInformationForIp;
static tx_state_t *tx_state = &global_tx_state;

void loadSigningUx();

UX_STEP_NOCB(
    ux_sign_public_info_for_ip_0_step,
    nn,
    {
      "Review",
      "transaction"
    });
UX_STEP_NOCB(
    ux_sign_public_info_for_ip_1_step,
    bn_paging,
    {
      .title = "Id Cred Pub",
      .text = (char *) global.signPublicInformationForIp.idCredPub
    });
UX_STEP_CB(
    ux_sign_public_info_for_ip_2_step,
    bn_paging,
    sendSuccessNoIdle(),
    {
      .title = "CredId",
      .text = (char *) global.signPublicInformationForIp.credId
    });
UX_FLOW(ux_sign_public_info_for_ip,
    &ux_sign_public_info_for_ip_0_step,
    &ux_sign_public_info_for_ip_1_step,
    &ux_sign_public_info_for_ip_2_step
);

UX_STEP_CB(
    ux_sign_public_info_for_i_public_key_0_step,
    bn_paging,
    sendSuccessNoIdle(),
    {
      .title = "Verification key",
      .text = (char *) global.signPublicInformationForIp.publicKey
    });
UX_FLOW(ux_sign_public_info_for_i_public_key,
    &ux_sign_public_info_for_i_public_key_0_step
);

UX_STEP_NOCB(
    ux_sign_public_info_for_ip_threshold_0_step,
    bn,
    {
      "Threshold",
      global.signPublicInformationForIp.threshold
    });
UX_STEP_CB(
    ux_sign_public_info_for_ip_threshold_1_step,
    pnn,
    buildAndSignTransactionHash(),
    {
      &C_icon_validate_14,
      "Sign",
      "transaction"
    });
UX_STEP_CB(
    ux_sign_public_info_for_ip_threshold_2_step,
    pnn,
    declineToSignTransaction(),
    {
      &C_icon_crossmark,
      "Decline to",
      "sign transaction"
    });
UX_FLOW(ux_sign_public_info_for_ip_threshold,
    &ux_sign_public_info_for_ip_threshold_0_step,
    &ux_sign_public_info_for_ip_threshold_1_step,
    &ux_sign_public_info_for_ip_threshold_2_step
);

#define P1_INITIAL              0x00
#define P1_VERIFICATION_KEY     0x01
#define P1_THRESHOLD            0x02

void handleSignPublicInformationForIp(uint8_t *dataBuffer, uint8_t p1, uint8_t dataLength, volatile unsigned int *flags) {
    if (p1 != P1_INITIAL && tx_state->initialized == false) {
        THROW(SW_INVALID_STATE);
    }
    
    if (p1 == P1_INITIAL) {
        int bytesRead = parseKeyDerivationPath(dataBuffer);
        dataBuffer += bytesRead;

        cx_sha256_init(&tx_state->hash);
        tx_state->initialized = true;

        // Parse id_cred_pub so it can be displayed.
        uint8_t idCredPub[48];
        os_memmove(idCredPub, dataBuffer, 48);
        cx_hash((cx_hash_t *) &tx_state->hash, 0, idCredPub, 48, NULL, 0);
        dataBuffer += 48;
        toHex(idCredPub, 48, ctx->idCredPub);

        // Parse cred_id so it can be displayed.
        uint8_t credId[48];
        os_memmove(credId, dataBuffer, 48);
        cx_hash((cx_hash_t *) &tx_state->hash, 0, credId, 48, NULL, 0);
        dataBuffer += 48;
        toHex(credId, 48, ctx->credId);

        // Parse number of public-keys that will be received next.
        ctx->publicKeysLength = dataBuffer[0];
        cx_hash((cx_hash_t *) &tx_state->hash, 0, dataBuffer, 1, NULL, 0);
        dataBuffer += 1;

        ctx->state = VERIFICATION_KEY;
        ux_flow_init(0, ux_sign_public_info_for_ip, NULL);
        *flags |= IO_ASYNCH_REPLY;
    } else if (p1 == P1_VERIFICATION_KEY) {
        if (ctx->publicKeysLength <= 0 || ctx->state != VERIFICATION_KEY) {
            THROW(SW_INVALID_STATE);
        }
        // Hash key index
        cx_hash((cx_hash_t *) &tx_state->hash, 0, dataBuffer, 1, NULL, 0);
        dataBuffer += 1;

        // Hash key type
        cx_hash((cx_hash_t *) &tx_state->hash, 0, dataBuffer, 1, NULL, 0);
        dataBuffer += 1;

        uint8_t publicKey[32];
        os_memmove(publicKey, dataBuffer, 32);
        cx_hash((cx_hash_t *) &tx_state->hash, 0, publicKey, 32, NULL, 0);
        dataBuffer += 32;
        toHex(publicKey, 32, ctx->publicKey);

        ctx->publicKeysLength -= 1;
        if (ctx->publicKeysLength == 0) {
            ctx->state = THRESHOLD;
        }
        ux_flow_init(0, ux_sign_public_info_for_i_public_key, NULL);
        *flags |= IO_ASYNCH_REPLY;
    } else if (p1 == P1_THRESHOLD) {
        if (ctx->state != THRESHOLD) {
            THROW(SW_INVALID_STATE);
        }

        // Read the threshold byte and parse it to display it.
        cx_hash((cx_hash_t *) &tx_state->hash, 0, dataBuffer, 1, NULL, 0);
        bin2dec(ctx->threshold, dataBuffer[0]);
        dataBuffer += 1;

        ux_flow_init(0, ux_sign_public_info_for_ip_threshold, NULL);
        *flags |= IO_ASYNCH_REPLY;
    } else {
        THROW(SW_INVALID_STATE);
    }
}
