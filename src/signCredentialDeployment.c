#include <os.h>
#include <os_io_seproxyhal.h>
#include "cx.h"
#include <stdint.h>
#include "menu.h"
#include "util.h"
#include <string.h>
#include "base58check.h"
#include <stdio.h>
#include "sign.h"

static accountSubtreePath_t *keyPath = &path;
static signCredentialDeploymentContext_t *ctx = &global.signCredentialDeploymentContext;

// TODO Move to shared memory, but it failed before. So keeping it here to make progress...
static cx_sha256_t attributeHash;
static tx_state_t *tx_state = &global_tx_state;

void processNextVerificationKey();
void signCredentialDeployment();
void declineToSignCredentialDeployment();

UX_STEP_CB(
    ux_credential_deployment_initial_flow_0_step,
    nn,
    sendSuccessNoIdle(0),
    {
      "Review",
      "transaction"
    });
UX_FLOW(ux_credential_deployment_initial_flow,
    &ux_credential_deployment_initial_flow_0_step
);

UX_STEP_CB(
    ux_credential_deployment_verification_key_flow_0_step,
    bnnn_paging,
    processNextVerificationKey(),
    {
      .title = "Verification key",
      .text = (char *) global.signCredentialDeploymentContext.accountVerificationKey
    });
UX_FLOW(ux_credential_deployment_verification_key_flow,
    &ux_credential_deployment_verification_key_flow_0_step
);

UX_STEP_NOCB(
    ux_credential_deployment_threshold_flow_0_step,
    bn,
    {
      "Sig threshold",
      (char *) global.signCredentialDeploymentContext.signatureThreshold
    });
UX_STEP_NOCB(
    ux_credential_deployment_threshold_flow_1_step,
    bn_paging,
    {
      "RegIdCred",
      (char *) global.signCredentialDeploymentContext.regIdCred
    });
UX_STEP_NOCB(
    ux_credential_deployment_threshold_flow_2_step,
    bn,
    {
      "Identity provider",
      (char *) global.signCredentialDeploymentContext.identityProviderIdentity
    });
UX_STEP_CB(
    ux_credential_deployment_threshold_flow_3_step,
    bn,
    sendSuccessNoIdle(0),
    {
      "Revocation threshold",
      (char *) global.signCredentialDeploymentContext.anonymityRevocationThreshold
    });
UX_FLOW(ux_credential_deployment_threshold_flow,
    &ux_credential_deployment_threshold_flow_0_step,
    &ux_credential_deployment_threshold_flow_1_step,
    &ux_credential_deployment_threshold_flow_2_step,
    &ux_credential_deployment_threshold_flow_3_step
);

UX_STEP_NOCB(
    ux_credential_deployment_aridentity_key_flow_0_step,
    bn,
    {
      "ArIdentity",
      (char *) global.signCredentialDeploymentContext.arIdentity
    });

// TODO Consider if it is necessary to show the encrypted id cred pub shares. I assume that the transaction is rejected
// TODO if they are invalid anyway? And the user has no real chance of validating it anyhow?
UX_STEP_CB(
    ux_credential_deployment_aridentity_key_flow_1_step,
    bn_paging,
    sendSuccessNoIdle(0),
    {
      "EncIdCred",
      (char *) global.signCredentialDeploymentContext.encIdCredPubShare
    });
UX_FLOW(ux_credential_deployment_aridentity_key_flow,
    &ux_credential_deployment_aridentity_key_flow_0_step,
    &ux_credential_deployment_aridentity_key_flow_1_step
);

UX_STEP_NOCB(
    ux_credential_deployment_dates_0_step,
    bn,
    {
      "Valid to",
      (char *) global.signCredentialDeploymentContext.validTo
    });
UX_STEP_CB(
    ux_credential_deployment_dates_1_step,
    bn,
    sendSuccessNoIdle(0),
    {
      "Created at",
      (char *) global.signCredentialDeploymentContext.createdAt
    });
UX_FLOW(ux_credential_deployment_dates,
    &ux_credential_deployment_dates_0_step,
    &ux_credential_deployment_dates_1_step
);

UX_STEP_CB(
    ux_credential_deployment_attributes_0_step,
    bn_paging,
    sendSuccessNoIdle(0),
    {
        "Attributes hash",
        (char *) global.signCredentialDeploymentContext.attributeHashDisplay
    });
UX_FLOW(ux_credential_deployment_attributes,
    &ux_credential_deployment_attributes_0_step
);

void processNextVerificationKey() {
    if (ctx->numberOfVerificationKeys == 0) {
        // TODO Update state here. That is why there are two branches here that currently do the same.
        sendSuccessNoIdle(0);
    } else {
        sendSuccessNoIdle(0);   // Request more data from the computer.
    }
}

void parseVerificationKey(uint8_t *buffer) {
    uint8_t verificationKey[32];
    os_memmove(verificationKey, buffer, 32);
    buffer += 32;
    cx_hash((cx_hash_t *) &tx_state->hash, 0, verificationKey, 32, NULL, 0);

    // Convert to a human-readable format.
    toHex(verificationKey, sizeof(verificationKey), ctx->accountVerificationKey);
    ctx->numberOfVerificationKeys -= 1;

    // Show to the user.
    ux_flow_init(0, ux_credential_deployment_verification_key_flow, NULL);
}

void parseVerificationKeysLength(uint8_t *dataBuffer) {
    // Read number of verification keys that we are going to receive.
    uint8_t numberOfVerificationKeysArray[1];
    os_memmove(numberOfVerificationKeysArray, dataBuffer, 1);
    ctx->numberOfVerificationKeys = numberOfVerificationKeysArray[0];

    cx_hash((cx_hash_t *) &tx_state->hash, 0, numberOfVerificationKeysArray, 1, NULL, 0);
    ux_flow_init(0, ux_credential_deployment_initial_flow, NULL);
}

// APDU parameters specific to credential deployment transaction (multiple packets protocol).
#define P1_INITIAL_PACKET           0x00    // Sent for 1st packet of the transfer.
#define P1_VERIFICATION_KEY         0x01    // Sent for packets containing a verification key.
#define P1_SIGNATURE_THRESHOLD      0x02    // Sent for the packet containing signature threshold, RegIdCred,
                                            // identity provider identity, anonymity invocation threshold
                                            // and the length of the anonymity revocation data.
#define P1_AR_IDENTITY              0x03    // Sent for the packets containing a aridentity / encidcredpubshares pair.
#define P1_CREDENTIAL_DATES         0x04    // Sent for the packet containing the credential valid to / create at dates.
#define P1_ATTRIBUTE_TAG            0x05    // Sent for the packet containing the attribute tag, and the attribute
                                            // value length, which is used to read the attribute value.
#define P1_ATTRIBUTE_VALUE          0x06    // Sent for the packet containing an attribute value.
#define P1_LENGTH_OF_PROOFS         0x07    // Sent for the packet containing the byte length of the proofs.
#define P1_PROOFS                   0x08    // Sent for the packets containing proof bytes.

// TODO Add improved state checking to disallow a computer stepping outside of the protocol.
void handleSignCredentialDeployment(uint8_t *dataBuffer, uint8_t p1, volatile unsigned int *flags) {
    if (p1 != P1_INITIAL_PACKET && tx_state->initialized == false) {
        THROW(SW_INVALID_STATE);
    }

    if (p1 == P1_INITIAL_PACKET) {
        parseAccountSignatureKeyPath(dataBuffer);
        dataBuffer += 2;

        os_memmove(ctx->displayAccount, "with #", 6);
        bin2dec(ctx->displayAccount + 6, keyPath->accountIndex);

        // Initialize values.
        cx_sha256_init(&tx_state->hash);
        tx_state->initialized = true;

        ctx->state = 1;
        parseVerificationKeysLength(dataBuffer);
    } else if (p1 == P1_VERIFICATION_KEY) {
        if (ctx->numberOfVerificationKeys > 0 && ctx->state == TX_VERIFICATION_KEY) {
            if (ctx->numberOfVerificationKeys == 0) {
                ctx->state += 1;
            }
            parseVerificationKey(dataBuffer);
        } else {
            THROW(0x6B01);
        }
    } else if (p1 == P1_SIGNATURE_THRESHOLD) {
        if (ctx->numberOfVerificationKeys != 0) {
            THROW(0x6B01);  // Invalid state, the sender has not sent all verification keys before moving on.
        }

        // Parse signature threshold.
        uint8_t temp[1];
        os_memmove(temp, dataBuffer, 1);
        bin2dec(ctx->signatureThreshold, temp[0]);
        dataBuffer += 1;
        cx_hash((cx_hash_t *) &tx_state->hash, 0, temp, 1, NULL, 0);

        // Parse RegIdCred and make it displayable as hex.
        uint8_t regIdCred[48];
        os_memmove(regIdCred, dataBuffer, 48);
        dataBuffer += 48;
        toHex(regIdCred, sizeof(regIdCred), ctx->regIdCred);
        cx_hash((cx_hash_t *) &tx_state->hash, 0, regIdCred, 48, NULL, 0);

        // Parse identity provider identity.
        uint8_t identityProviderIdentity[4];
        os_memmove(identityProviderIdentity, dataBuffer, sizeof(identityProviderIdentity));
        uint32_t identityProviderValue = U4BE(identityProviderIdentity, 0);
        bin2dec(ctx->identityProviderIdentity, identityProviderValue);
        dataBuffer += 4;
        cx_hash((cx_hash_t *) &tx_state->hash, 0, identityProviderIdentity, 4, NULL, 0);

        // Parse anonymity revocation threshold.
        os_memmove(temp, dataBuffer, 1);
        bin2dec(ctx->anonymityRevocationThreshold, temp[0]);
        dataBuffer += 1;
        cx_hash((cx_hash_t *) &tx_state->hash, 0, temp, 1, NULL, 0);

        // Parse the length of the following list of anonymity revokers.
        ctx->anonymityRevocationListLength = U2BE(dataBuffer, 0);
        cx_hash((cx_hash_t *) &tx_state->hash, 0, dataBuffer, 2, NULL, 0);
        dataBuffer += 2;

        // Initialize values for later.
        cx_sha256_init(&attributeHash);

        // Display the loaded data.
        ux_flow_init(0, ux_credential_deployment_threshold_flow, NULL);
    } else if (p1 == P1_AR_IDENTITY) {
        if (ctx->anonymityRevocationListLength <= 0) {
            THROW(0x6B01);  // Invalid state, sender says ar identity pair is incoming, but we already received all.
        }

        // Parse ArIdentity
        uint32_t arIdentity = U4BE(dataBuffer, 0);
        bin2dec(ctx->arIdentity, arIdentity);
        cx_hash((cx_hash_t *) &tx_state->hash, 0, dataBuffer, 4, NULL, 0);
        dataBuffer += 4;

        // Parse enc_id_cred_pub_share
        uint8_t encIdCredPubShare[96];
        os_memmove(encIdCredPubShare, dataBuffer, 96);
        toHex(encIdCredPubShare, sizeof(encIdCredPubShare), ctx->encIdCredPubShare);
        cx_hash((cx_hash_t *) &tx_state->hash, 0, encIdCredPubShare, 96, NULL, 0);
        dataBuffer += 96;

        // Display the loaded data.
        ux_flow_init(0, ux_credential_deployment_aridentity_key_flow, NULL);
    } else if (p1 == P1_CREDENTIAL_DATES) {
        uint8_t temp[1];

        // Build display of valid to
        uint16_t validToYear = U2BE(dataBuffer, 0);
        cx_hash((cx_hash_t *) &tx_state->hash, 0, dataBuffer, 2, NULL, 0);
        dataBuffer += 2;
        os_memmove(temp, dataBuffer, 1);
        cx_hash((cx_hash_t *) &tx_state->hash, 0, dataBuffer, 1, NULL, 0);
        dataBuffer += 1;
        bin2dec(ctx->validTo, validToYear);
        ctx->validTo[4] = ' ';
        bin2dec(ctx->validTo + 5, temp[0]);

        // Build display of created at
        uint16_t createdAtYear = U2BE(dataBuffer, 0);
        cx_hash((cx_hash_t *) &tx_state->hash, 0, dataBuffer, 2, NULL, 0);
        dataBuffer += 2;
        os_memmove(temp, dataBuffer, 1);
        cx_hash((cx_hash_t *) &tx_state->hash, 0, dataBuffer, 1, NULL, 0);
        dataBuffer += 1;
        bin2dec(ctx->createdAt, createdAtYear);
        ctx->createdAt[4] = ' ';
        bin2dec(ctx->createdAt + 5, temp[0]);

        // Read attribute list length
        ctx->attributeListLength = U2BE(dataBuffer, 0);
        cx_hash((cx_hash_t *) &tx_state->hash, 0, dataBuffer, 2, NULL, 0);
        dataBuffer += 2;

        ux_flow_init(0, ux_credential_deployment_dates, NULL);
    } else if (p1 == P1_ATTRIBUTE_TAG) {
        if (ctx->attributeListLength <= 0) {
            THROW(0x6B01);
        }

        // Parse attribute tag, and map it the attribute name (the display text).
        uint8_t attributeTag[1];
        os_memmove(attributeTag, dataBuffer, 1);
        dataBuffer += 1;
        cx_hash((cx_hash_t *) &attributeHash, 0, attributeTag, 1, NULL, 0);
        cx_hash((cx_hash_t *) &tx_state->hash, 0, attributeTag, 1, NULL, 0);


        // Parse attribute length, so we know how much to parse in next packet.
        uint8_t attributeValueLength[1];
        os_memmove(attributeValueLength, dataBuffer, 1);
        ctx->attributeValueLength = attributeValueLength[0];
        cx_hash((cx_hash_t *) &attributeHash, 0, attributeValueLength, 1, NULL, 0);
        cx_hash((cx_hash_t *) &tx_state->hash, 0, attributeValueLength, 1, NULL, 0);

        // Ask computer for the attribute value.
        sendSuccessNoIdle(0);
    } else if (p1 == P1_ATTRIBUTE_VALUE) {
        // Add attribute value to the hash.
        cx_hash((cx_hash_t *) &attributeHash, 0, dataBuffer, ctx->attributeValueLength, NULL, 0);
        cx_hash((cx_hash_t *) &tx_state->hash, 0, dataBuffer, ctx->attributeValueLength, NULL, 0);

        ctx->attributeListLength -= 1;

        // We have processed all attributes, so display the attribute hash value.
        if (ctx->attributeListLength == 0) {
            uint8_t attributeHashBytes[32];
            cx_hash((cx_hash_t *) &attributeHash, CX_LAST, NULL, 0, attributeHashBytes, 32);
            toHex(attributeHashBytes, sizeof(attributeHashBytes), ctx->attributeHashDisplay);
            ux_flow_init(0, ux_credential_deployment_attributes, NULL);
        } else {
            // There are additional attributes to be read, so ask for more.
            sendSuccessNoIdle(0);
        }
    } else if (p1 == P1_LENGTH_OF_PROOFS) {
        ctx->proofLength = U4BE(dataBuffer, 0);
        cx_hash((cx_hash_t *) &tx_state->hash, 0, dataBuffer, 4, NULL, 0);
        sendSuccessNoIdle(0);
    } else if (p1 == P1_PROOFS) {
        if (ctx->proofLength > 255) {
            cx_hash((cx_hash_t *) &tx_state->hash, 0, dataBuffer, 255, NULL, 0);
            os_memmove(ctx->buffer, dataBuffer, 255);
            ctx->proofLength -= 255;
            sendSuccessNoIdle(0);
        } else {
            cx_hash((cx_hash_t *) &tx_state->hash, 0, dataBuffer, ctx->proofLength, NULL, 0);
            os_memmove(ctx->buffer, dataBuffer, ctx->proofLength);
            ux_flow_init(0, ux_sign_flow_shared, NULL);
        }
    }

    *flags |= IO_ASYNCH_REPLY;
}
