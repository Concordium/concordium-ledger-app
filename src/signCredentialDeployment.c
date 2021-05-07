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

static signCredentialDeploymentContext_t *ctx = &global.signCredentialDeploymentContext;
static cx_sha256_t attributeHash;
static tx_state_t *tx_state = &global_tx_state;

void processNextVerificationKey();
void signCredentialDeployment();
void declineToSignCredentialDeployment();
void handleSignCredentialDeployment(uint8_t *dataBuffer, uint8_t p1, uint8_t p2, volatile unsigned int *flags);

UX_STEP_CB(
    ux_credential_deployment_initial_flow_0_step,
    nn,
    sendSuccessNoIdle(),
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
    sendSuccessNoIdle(),
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
    sendSuccessNoIdle(),
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
    sendSuccessNoIdle(),
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
    sendSuccessNoIdle(),
    {
        "Attributes hash",
        (char *) global.signCredentialDeploymentContext.attributeHashDisplay
    });
UX_FLOW(ux_credential_deployment_attributes,
    &ux_credential_deployment_attributes_0_step
);

UX_STEP_NOCB(
    ux_sign_credential_deployment_0_step,
    bn_paging,
    {
      .title = "Account address",
      .text = (char *) global.signCredentialDeploymentContext.accountAddress
    });
UX_STEP_CB(
    ux_sign_credential_deployment_1_step,
    pnn,
    buildAndSignTransactionHash(),
    {
      &C_icon_validate_14,
      "Sign",
      "transaction"
    });
UX_STEP_CB(
    ux_sign_credential_deployment_2_step,
    pnn,
    declineToSignTransaction(),
    {
      &C_icon_crossmark,
      "Decline to",
      "sign transaction"
    });
UX_FLOW(ux_sign_credential_deployment,
    &ux_sign_credential_deployment_0_step,
    &ux_sign_credential_deployment_1_step,
    &ux_sign_credential_deployment_2_step
);


UX_STEP_CB(
    ux_sign_credential_update_id_0_step,
    bn_paging,
    sendSuccessNoIdle(),
    {
      .title = "Remove CredId",
      .text = (char *) global.signCredentialDeploymentContext.credentialId
    });
UX_FLOW(ux_sign_credential_update_id,
    &ux_sign_credential_update_id_0_step
);


/*
 * The UI flow for the final part of the update credential, which displays
 * the threshold and allows the user to either sign or decline.
 */
UX_STEP_NOCB(
    ux_sign_credential_update_threshold_0_step,
    bn_paging,
    {
      .title = "Threshold",
      .text = (char *) global.signCredentialDeploymentContext.threshold
    });
UX_STEP_CB(
    ux_sign_credential_update_threshold_1_step,
    pnn,
    buildAndSignTransactionHash(),
    {
      &C_icon_validate_14,
      "Sign",
      "transaction"
    });
UX_STEP_CB(
    ux_sign_credential_update_threshold_2_step,
    pnn,
    declineToSignTransaction(),
    {
      &C_icon_crossmark,
      "Decline to",
      "sign transaction"
    });
UX_FLOW(ux_sign_credential_update_threshold,
    &ux_sign_credential_update_threshold_0_step,
    &ux_sign_credential_update_threshold_1_step,
    &ux_sign_credential_update_threshold_2_step
);

void processNextVerificationKey() {
    if (ctx->numberOfVerificationKeys == 0) {
        // TODO Update state here. That is why there are two branches here that currently do the same.
        sendSuccessNoIdle();
    } else {
        sendSuccessNoIdle();   // Request more data from the computer.
    }
}

void parseVerificationKey(uint8_t *buffer) {
    // Hash key index
    cx_hash((cx_hash_t *) &tx_state->hash, 0, buffer, 1, NULL, 0);
    buffer += 1;

    // Hash schemeId
    cx_hash((cx_hash_t *) &tx_state->hash, 0, buffer, 1, NULL, 0);
    buffer += 1;

    uint8_t verificationKey[32];
    memmove(verificationKey, buffer, 32);
    buffer += 32;
    cx_hash((cx_hash_t *) &tx_state->hash, 0, verificationKey, 32, NULL, 0);

    // Convert to a human-readable format.
    toHex(verificationKey, sizeof(verificationKey), ctx->accountVerificationKey);
    ctx->numberOfVerificationKeys -= 1;

    // Show to the user.
    ux_flow_init(0, ux_credential_deployment_verification_key_flow, NULL);
}

// APDU parameters specific to credential deployment transaction (multiple packets protocol).
#define P1_INITIAL_PACKET           0x00    // Sent for 1st packet of the transfer.
#define P1_VERIFICATION_KEY_LENGTH  0x0A    // TODO: Move to 0x02
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
#define P1_NEW_OR_EXISTING          0x09

#define P2_CREDENTIAL_INITIAL               0x00
#define P2_CREDENTIAL_CREDENTIAL_INDEX      0x01
#define P2_CREDENTIAL_CREDENTIAL            0x02
#define P2_CREDENTIAL_ID_COUNT              0x03
#define P2_CREDENTIAL_ID                    0x04
#define P2_THRESHOLD                        0x05

void handleSignUpdateCredential(uint8_t *dataBuffer, uint8_t p1, uint8_t p2, volatile unsigned int *flags) {
    if (p2 != P2_CREDENTIAL_INITIAL && tx_state->initialized == false) {
        THROW(SW_INVALID_STATE);
    }

    if (p2 == P2_CREDENTIAL_INITIAL) {
        int bytesRead = parseKeyDerivationPath(dataBuffer);
        dataBuffer += bytesRead;

        cx_sha256_init(&tx_state->hash);
        tx_state->initialized = true;

        cx_hash((cx_hash_t *) &tx_state->hash, 0, dataBuffer, ACCOUNT_TRANSACTION_HEADER_LENGTH, NULL, 0);
        dataBuffer += ACCOUNT_TRANSACTION_HEADER_LENGTH;
        uint8_t transactionKind = dataBuffer[0];
        if (transactionKind != UPDATE_CREDENTIALS) {
            THROW(SW_INVALID_TRANSACTION);
        }
        cx_hash((cx_hash_t *) &tx_state->hash, 0, dataBuffer, 1, NULL, 0);
        dataBuffer += 1;

        ctx->credentialDeploymentCount = dataBuffer[0];
        cx_hash((cx_hash_t *) &tx_state->hash, 0, dataBuffer, 1, NULL, 0);
        if (ctx->credentialDeploymentCount == 0) {
            ctx->updateCredentialState = TX_UPDATE_CREDENTIAL_ID_COUNT;
        } else {
            ctx->updateCredentialState = TX_UPDATE_CREDENTIAL_CREDENTIAL_INDEX;
        }

        ux_flow_init(0, ux_credential_deployment_initial_flow, NULL);
        *flags |= IO_ASYNCH_REPLY;
    } else if (p2 == P2_CREDENTIAL_CREDENTIAL_INDEX && ctx->updateCredentialState == TX_UPDATE_CREDENTIAL_CREDENTIAL_INDEX && ctx->credentialDeploymentCount > 0) {
        // Add the credential index to the hash
        cx_hash((cx_hash_t *) &tx_state->hash, 0, dataBuffer, 1, NULL, 0);
        ctx->updateCredentialState = TX_UPDATE_CREDENTIAL_CREDENTIAL;
        sendSuccessNoIdle();
    } else if (p2 == P2_CREDENTIAL_CREDENTIAL && ctx->updateCredentialState == TX_UPDATE_CREDENTIAL_CREDENTIAL && ctx->credentialDeploymentCount > 0) {
        handleSignCredentialDeployment(dataBuffer, p1, p2, flags);
    } else if (p2 == P2_CREDENTIAL_ID_COUNT && ctx->updateCredentialState == TX_UPDATE_CREDENTIAL_ID_COUNT) {
        ctx->credentialIdCount = dataBuffer[0];
        cx_hash((cx_hash_t *) &tx_state->hash, 0, dataBuffer, 1, NULL, 0);

        if (ctx->credentialIdCount == 0) {
            ctx->updateCredentialState = TX_UPDATE_CREDENTIAL_THRESHOLD;
        } else {
            ctx->updateCredentialState = TX_UPDATE_CREDENTIAL_ID;
        }
        sendSuccessNoIdle();
    } else if (p2 == P2_CREDENTIAL_ID && ctx->updateCredentialState == TX_UPDATE_CREDENTIAL_ID) {
        cx_hash((cx_hash_t *) &tx_state->hash, 0, dataBuffer, 48, NULL, 0);
        toHex(dataBuffer, 48, ctx->credentialId);

        ctx->credentialIdCount -= 1;
        if (ctx->credentialIdCount == 0) {
            ctx->updateCredentialState = TX_UPDATE_CREDENTIAL_THRESHOLD;
        }
        
        ux_flow_init(0, ux_sign_credential_update_id, NULL);
        *flags |= IO_ASYNCH_REPLY;
    } else if (p2 == P2_THRESHOLD && ctx->updateCredentialState == TX_UPDATE_CREDENTIAL_THRESHOLD) {
        uint8_t threshold = dataBuffer[0];
        bin2dec(ctx->threshold, threshold);
        cx_hash((cx_hash_t *) &tx_state->hash, 0, dataBuffer, 1, NULL, 0);

        ux_flow_init(0, ux_sign_credential_update_threshold, NULL);
        *flags |= IO_ASYNCH_REPLY;
    } else {
        THROW(SW_INVALID_STATE);
    }
}

// TODO Add improved state checking to disallow a computer stepping outside of the protocol.
void handleSignCredentialDeployment(uint8_t *dataBuffer, uint8_t p1, uint8_t p2, volatile unsigned int *flags) {
    if (p1 != P1_INITIAL_PACKET && tx_state->initialized == false) {
        THROW(SW_INVALID_STATE);
    }

    if (p1 == P1_INITIAL_PACKET) {
        int bytesRead = parseKeyDerivationPath(dataBuffer);
        dataBuffer += bytesRead;

        // Initialize values.
        cx_sha256_init(&tx_state->hash);
        tx_state->initialized = true;
        ctx->state = 1;

        ux_flow_init(0, ux_credential_deployment_initial_flow, NULL);
    } else if (p1 == P1_VERIFICATION_KEY_LENGTH) {
        ctx->numberOfVerificationKeys = dataBuffer[0];
        cx_hash((cx_hash_t *) &tx_state->hash, 0, dataBuffer, 1, NULL, 0);
        ctx->state = 1;
        sendSuccessNoIdle();
    } else if (p1 == P1_VERIFICATION_KEY) {
        if (ctx->numberOfVerificationKeys > 0 && ctx->state == TX_VERIFICATION_KEY) {
            if (ctx->numberOfVerificationKeys == 0) {
                ctx->state += 1;
            }
            parseVerificationKey(dataBuffer);
        } else {
            THROW(SW_INVALID_STATE);
        }
    } else if (p1 == P1_SIGNATURE_THRESHOLD) {
        if (ctx->numberOfVerificationKeys != 0) {
            THROW(SW_INVALID_STATE);  // Invalid state, the sender has not sent all verification keys before moving on.
        }

        // Parse signature threshold.
        uint8_t temp[1];
        memmove(temp, dataBuffer, 1);
        bin2dec(ctx->signatureThreshold, temp[0]);
        dataBuffer += 1;
        cx_hash((cx_hash_t *) &tx_state->hash, 0, temp, 1, NULL, 0);

        // Parse RegIdCred and make it displayable as hex.
        uint8_t regIdCred[48];
        memmove(regIdCred, dataBuffer, 48);
        dataBuffer += 48;
        toHex(regIdCred, sizeof(regIdCred), ctx->regIdCred);
        cx_hash((cx_hash_t *) &tx_state->hash, 0, regIdCred, 48, NULL, 0);

        // Parse identity provider identity.
        uint8_t identityProviderIdentity[4];
        memmove(identityProviderIdentity, dataBuffer, sizeof(identityProviderIdentity));
        uint32_t identityProviderValue = U4BE(identityProviderIdentity, 0);
        bin2dec(ctx->identityProviderIdentity, identityProviderValue);
        dataBuffer += 4;
        cx_hash((cx_hash_t *) &tx_state->hash, 0, identityProviderIdentity, 4, NULL, 0);

        // Parse anonymity revocation threshold.
        memmove(temp, dataBuffer, 1);
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
            THROW(SW_INVALID_STATE);  // Invalid state, sender says ar identity pair is incoming, but we already received all.
        }

        // Parse ArIdentity
        uint32_t arIdentity = U4BE(dataBuffer, 0);
        bin2dec(ctx->arIdentity, arIdentity);
        cx_hash((cx_hash_t *) &tx_state->hash, 0, dataBuffer, 4, NULL, 0);
        dataBuffer += 4;

        // Parse enc_id_cred_pub_share
        uint8_t encIdCredPubShare[96];
        memmove(encIdCredPubShare, dataBuffer, 96);
        toHex(encIdCredPubShare, sizeof(encIdCredPubShare), ctx->encIdCredPubShare);
        cx_hash((cx_hash_t *) &tx_state->hash, 0, encIdCredPubShare, 96, NULL, 0);
        dataBuffer += 96;

        sendSuccessNoIdle();
        // Display the loaded data.
        // ux_flow_init(0, ux_credential_deployment_aridentity_key_flow, NULL);
    } else if (p1 == P1_CREDENTIAL_DATES) {
        uint8_t temp[1];

        // Build display of valid to
        uint16_t validToYear = U2BE(dataBuffer, 0);
        cx_hash((cx_hash_t *) &tx_state->hash, 0, dataBuffer, 2, NULL, 0);
        dataBuffer += 2;
        memmove(temp, dataBuffer, 1);
        cx_hash((cx_hash_t *) &tx_state->hash, 0, dataBuffer, 1, NULL, 0);
        dataBuffer += 1;
        numberToText(ctx->validTo, validToYear);
        ctx->validTo[4] = ' ';
        bin2dec(ctx->validTo + 5, temp[0]);

        // Build display of created at
        uint16_t createdAtYear = U2BE(dataBuffer, 0);
        cx_hash((cx_hash_t *) &tx_state->hash, 0, dataBuffer, 2, NULL, 0);
        dataBuffer += 2;
        memmove(temp, dataBuffer, 1);
        cx_hash((cx_hash_t *) &tx_state->hash, 0, dataBuffer, 1, NULL, 0);
        dataBuffer += 1;
        numberToText(ctx->createdAt, createdAtYear);
        ctx->createdAt[4] = ' ';
        bin2dec(ctx->createdAt + 5, temp[0]);

        // Read attribute list length
        ctx->attributeListLength = U2BE(dataBuffer, 0);
        cx_hash((cx_hash_t *) &tx_state->hash, 0, dataBuffer, 2, NULL, 0);
        dataBuffer += 2;

        ux_flow_init(0, ux_credential_deployment_dates, NULL);
    } else if (p1 == P1_ATTRIBUTE_TAG) {
        if (ctx->attributeListLength <= 0) {
            THROW(SW_INVALID_STATE);
        }

        // Parse attribute tag, and map it the attribute name (the display text).
        uint8_t attributeTag[1];
        memmove(attributeTag, dataBuffer, 1);
        dataBuffer += 1;
        cx_hash((cx_hash_t *) &attributeHash, 0, attributeTag, 1, NULL, 0);
        cx_hash((cx_hash_t *) &tx_state->hash, 0, attributeTag, 1, NULL, 0);

        // Parse attribute length, so we know how much to parse in next packet.
        uint8_t attributeValueLength[1];
        memmove(attributeValueLength, dataBuffer, 1);
        ctx->attributeValueLength = attributeValueLength[0];
        cx_hash((cx_hash_t *) &attributeHash, 0, attributeValueLength, 1, NULL, 0);
        cx_hash((cx_hash_t *) &tx_state->hash, 0, attributeValueLength, 1, NULL, 0);

        // Ask computer for the attribute value.
        sendSuccessNoIdle();
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
            sendSuccessNoIdle();
        }
    } else if (p1 == P1_LENGTH_OF_PROOFS) {
        ctx->proofLength = U4BE(dataBuffer, 0);
        if (p2 == P2_CREDENTIAL_CREDENTIAL) {
            cx_hash((cx_hash_t *) &tx_state->hash, 0, dataBuffer, 4, NULL, 0);
        }

        sendSuccessNoIdle();
    } else if (p1 == P1_PROOFS) {
        if (ctx->proofLength > MAX_CDATA_LENGTH) {
            cx_hash((cx_hash_t *) &tx_state->hash, 0, dataBuffer, MAX_CDATA_LENGTH, NULL, 0);
            ctx->proofLength -= MAX_CDATA_LENGTH;
            sendSuccessNoIdle();
        } else {
            cx_hash((cx_hash_t *) &tx_state->hash, 0, dataBuffer, ctx->proofLength, NULL, 0);

            // If an update credential transaction, then update state to next step.
            if (p2 == P2_CREDENTIAL_CREDENTIAL && ctx->updateCredentialState == TX_UPDATE_CREDENTIAL_CREDENTIAL && ctx->credentialDeploymentCount > 0) {
                ctx->credentialDeploymentCount -= 1;
                if (ctx->credentialDeploymentCount == 0) {
                    ctx->updateCredentialState = TX_UPDATE_CREDENTIAL_ID_COUNT;
                    ctx->state = 0;
                } else {
                    ctx->updateCredentialState = TX_UPDATE_CREDENTIAL_CREDENTIAL_INDEX;
                }
            }
            sendSuccessNoIdle();
        }
    } else if (p1 == P1_NEW_OR_EXISTING) {
        // 0 indicates new, 1 indicates existing
        uint8_t newOrExisting = dataBuffer[0];
        cx_hash((cx_hash_t *) &tx_state->hash, 0, dataBuffer, 1, NULL, 0);
        dataBuffer += 1;

        if (newOrExisting == 0) {
            cx_hash((cx_hash_t *) &tx_state->hash, 0, dataBuffer, 8, NULL, 0);
            ux_flow_init(0, ux_sign_flow_shared, NULL);
        } else if (newOrExisting == 1) {
            uint8_t accountAddress[32];
            memmove(accountAddress, dataBuffer, 32);

            // Used to display account address.
            size_t outputSize = sizeof(ctx->accountAddress);
            if (base58check_encode(accountAddress, sizeof(accountAddress), ctx->accountAddress, &outputSize) != 0) {
            // The received address bytes are not a valid base58 encoding.
                THROW(SW_INVALID_TRANSACTION);  
            }
            ctx->accountAddress[50] = '\0';

            cx_hash((cx_hash_t *) &tx_state->hash, 0, dataBuffer, 32, NULL, 0);
            ux_flow_init(0, ux_sign_credential_deployment, NULL);
        } else {
            THROW(SW_INVALID_TRANSACTION);
        }
    }

    *flags |= IO_ASYNCH_REPLY;
}
