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
#include "accountSenderView.h"
#include "responseCodes.h"

static signCredentialDeploymentContext_t *ctx = &global.signCredentialDeploymentContext;
static cx_sha256_t attributeHash;
static tx_state_t *tx_state = &global_tx_state;

void processNextVerificationKey();
void signCredentialDeployment();
void declineToSignCredentialDeployment();
void handleSignCredentialDeployment(uint8_t *dataBuffer, uint8_t p1, uint8_t p2, volatile unsigned int *flags, bool isInitialCall);

UX_STEP_CB(
    ux_credential_deployment_initial_flow_0_step,
    nn,
    sendSuccessNoIdle(),
    {
      "Review",
      "details"
    });
UX_STEP_CB(
    ux_update_credentials_initial_flow_1_step,
    nn,
    sendSuccessNoIdle(),
    {
      "Continue",
      "with transaction"
    });
UX_FLOW(ux_credential_deployment_initial_flow,
    &ux_credential_deployment_initial_flow_0_step
);

UX_FLOW(ux_update_credentials_initial_flow,
    &ux_sign_flow_shared_review,
    &ux_sign_flow_account_sender_view,
    &ux_update_credentials_initial_flow_1_step
);

UX_STEP_CB(
    ux_credential_deployment_verification_key_flow_0_step,
    bnnn_paging,
    processNextVerificationKey(),
    {
      .title = "Public key",
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
    bnnn_paging,
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
      "Revoke threshold",
      (char *) global.signCredentialDeploymentContext.anonymityRevocationThreshold
    });
UX_FLOW(ux_credential_deployment_threshold_flow,
    &ux_credential_deployment_threshold_flow_0_step,
    &ux_credential_deployment_threshold_flow_1_step,
    &ux_credential_deployment_threshold_flow_2_step,
    &ux_credential_deployment_threshold_flow_3_step
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

UX_STEP_NOCB(
    ux_sign_credential_deployment_0_step,
    bnnn_paging,
    {
      .title = "Address",
      .text = (char *) global.signCredentialDeploymentContext.accountAddress
    });
UX_STEP_CB(
    ux_sign_credential_deployment_1_step,
    pnn,
    buildAndSignTransactionHash(),
    {
      &C_icon_validate_14,
      "Sign",
      "details"
    });
UX_STEP_CB(
    ux_sign_credential_deployment_2_step,
    pnn,
    sendUserRejection(),
    {
      &C_icon_crossmark,
      "Decline to",
      "sign details"
    });

UX_FLOW(ux_sign_credential_deployment_existing,
    &ux_sign_credential_deployment_0_step,
    &ux_sign_credential_deployment_1_step,
    &ux_sign_credential_deployment_2_step
);

UX_FLOW(ux_sign_credential_deployment_new,
    &ux_sign_credential_deployment_1_step,
    &ux_sign_credential_deployment_2_step
    );

UX_STEP_CB(
    ux_sign_credential_update_id_0_step,
    bnnn_paging,
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
    bnnn_paging,
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
    sendUserRejection(),
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
        ctx->state = TX_CREDENTIAL_DEPLOYMENT_SIGNATURE_THRESHOLD;
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
    cx_hash((cx_hash_t *) &tx_state->hash, 0, verificationKey, 32, NULL, 0);

    // Convert to a human-readable format.
    toPaginatedHex(verificationKey, sizeof(verificationKey), ctx->accountVerificationKey);
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

void handleSignUpdateCredential(uint8_t *dataBuffer, uint8_t p1, uint8_t p2, volatile unsigned int *flags, bool isInitialCall) {
    if (isInitialCall) {
        ctx->updateCredentialState = TX_UPDATE_CREDENTIAL_INITIAL;
        ctx->state = TX_CREDENTIAL_DEPLOYMENT_VERIFICATION_KEYS_LENGTH;
    }

    if (p2 == P2_CREDENTIAL_INITIAL && ctx->updateCredentialState == TX_UPDATE_CREDENTIAL_INITIAL) {
        int bytesRead = parseKeyDerivationPath(dataBuffer);
        dataBuffer += bytesRead;

        cx_sha256_init(&tx_state->hash);
        dataBuffer += hashAccountTransactionHeaderAndKind(dataBuffer, UPDATE_CREDENTIALS);

        ctx->credentialDeploymentCount = dataBuffer[0];
        cx_hash((cx_hash_t *) &tx_state->hash, 0, dataBuffer, 1, NULL, 0);
        if (ctx->credentialDeploymentCount == 0) {
            ctx->updateCredentialState = TX_UPDATE_CREDENTIAL_ID_COUNT;
        } else {
            ctx->updateCredentialState = TX_UPDATE_CREDENTIAL_CREDENTIAL_INDEX;
        }

        ux_flow_init(0, ux_update_credentials_initial_flow, NULL);
        *flags |= IO_ASYNCH_REPLY;
    } else if (p2 == P2_CREDENTIAL_CREDENTIAL_INDEX && ctx->updateCredentialState == TX_UPDATE_CREDENTIAL_CREDENTIAL_INDEX && ctx->credentialDeploymentCount > 0) {
        // Add the credential index to the hash
        cx_hash((cx_hash_t *) &tx_state->hash, 0, dataBuffer, 1, NULL, 0);
        ctx->updateCredentialState = TX_UPDATE_CREDENTIAL_CREDENTIAL;
        sendSuccessNoIdle();
    } else if (p2 == P2_CREDENTIAL_CREDENTIAL && ctx->updateCredentialState == TX_UPDATE_CREDENTIAL_CREDENTIAL && ctx->credentialDeploymentCount > 0) {
        handleSignCredentialDeployment(dataBuffer, p1, p2, flags, false);
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
        toPaginatedHex(dataBuffer, 48, ctx->credentialId);

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
        THROW(ERROR_INVALID_STATE);
    }
}

void handleSignCredentialDeployment(uint8_t *dataBuffer, uint8_t p1, uint8_t p2, volatile unsigned int *flags, bool isInitialCall) {
    if (isInitialCall) {
        ctx->state = TX_CREDENTIAL_DEPLOYMENT_INITIAL;
    }

    if (p1 == P1_INITIAL_PACKET && ctx->state == TX_CREDENTIAL_DEPLOYMENT_INITIAL) {
        parseKeyDerivationPath(dataBuffer);

        // Initialize values.
        cx_sha256_init(&tx_state->hash);
        ctx->state = TX_CREDENTIAL_DEPLOYMENT_VERIFICATION_KEYS_LENGTH;

        ux_flow_init(0, ux_credential_deployment_initial_flow, NULL);
    } else if (p1 == P1_VERIFICATION_KEY_LENGTH && ctx->state == TX_CREDENTIAL_DEPLOYMENT_VERIFICATION_KEYS_LENGTH) {
        ctx->numberOfVerificationKeys = dataBuffer[0];
        cx_hash((cx_hash_t *) &tx_state->hash, 0, dataBuffer, 1, NULL, 0);
        ctx->state = TX_CREDENTIAL_DEPLOYMENT_VERIFICATION_KEY;
        sendSuccessNoIdle();
    } else if (p1 == P1_VERIFICATION_KEY) {
        if (ctx->numberOfVerificationKeys > 0 && ctx->state == TX_CREDENTIAL_DEPLOYMENT_VERIFICATION_KEY) {
            parseVerificationKey(dataBuffer);
        } else {
            THROW(ERROR_INVALID_STATE);
        }
    } else if (p1 == P1_SIGNATURE_THRESHOLD && ctx->state == TX_CREDENTIAL_DEPLOYMENT_SIGNATURE_THRESHOLD) {
        if (ctx->numberOfVerificationKeys != 0) {
            THROW(ERROR_INVALID_STATE);  // Invalid state, the sender has not sent all verification keys before moving on.
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
        toPaginatedHex(regIdCred, sizeof(regIdCred), ctx->regIdCred);
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

        // Initialize values for later.
        cx_sha256_init(&attributeHash);

        ctx->state = TX_CREDENTIAL_DEPLOYMENT_AR_IDENTITY;

        // Display the loaded data.
        ux_flow_init(0, ux_credential_deployment_threshold_flow, NULL);
    } else if (p1 == P1_AR_IDENTITY && ctx->state == TX_CREDENTIAL_DEPLOYMENT_AR_IDENTITY) {
        if (ctx->anonymityRevocationListLength == 0) {
             // Invalid state, sender says ar identity pair is incoming, but we already received all.
            THROW(ERROR_INVALID_STATE);
        }

        // Parse ArIdentity
        uint32_t arIdentity = U4BE(dataBuffer, 0);
        bin2dec(ctx->arIdentity, arIdentity);
        cx_hash((cx_hash_t *) &tx_state->hash, 0, dataBuffer, 4, NULL, 0);
        dataBuffer += 4;

        // Parse enc_id_cred_pub_share
        uint8_t encIdCredPubShare[96];
        memmove(encIdCredPubShare, dataBuffer, 96);
        toPaginatedHex(encIdCredPubShare, sizeof(encIdCredPubShare), ctx->encIdCredPubShare);
        cx_hash((cx_hash_t *) &tx_state->hash, 0, encIdCredPubShare, 96, NULL, 0);

        if (ctx->anonymityRevocationListLength == 1) {
            ctx->state = TX_CREDENTIAL_DEPLOYMENT_CREDENTIAL_DATES;
        }
        ctx->anonymityRevocationListLength -= 1;
        sendSuccessNoIdle();

        // We do not show encrypted shares, as they are not possible for a user
        // to validate.
    } else if (p1 == P1_CREDENTIAL_DATES && ctx->state == TX_CREDENTIAL_DEPLOYMENT_CREDENTIAL_DATES) {
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
        
        if (ctx->attributeListLength == 0) {
            ctx->state = TX_CREDENTIAL_DEPLOYMENT_LENGTH_OF_PROOFS;
        } else {
            ctx->state = TX_CREDENTIAL_DEPLOYMENT_ATTRIBUTE_TAG;
        }

        ux_flow_init(0, ux_credential_deployment_dates, NULL);
    } else if (p1 == P1_ATTRIBUTE_TAG && ctx->state == TX_CREDENTIAL_DEPLOYMENT_ATTRIBUTE_TAG) {
        if (ctx->attributeListLength <= 0) {
            THROW(ERROR_INVALID_STATE);
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

        ctx->state = TX_CREDENTIAL_DEPLOYMENT_ATTRIBUTE_VALUE;
        // Ask computer for the attribute value.
        sendSuccessNoIdle();
    } else if (p1 == P1_ATTRIBUTE_VALUE && ctx->state == TX_CREDENTIAL_DEPLOYMENT_ATTRIBUTE_VALUE) {
        // Add attribute value to the hash.
        cx_hash((cx_hash_t *) &attributeHash, 0, dataBuffer, ctx->attributeValueLength, NULL, 0);
        cx_hash((cx_hash_t *) &tx_state->hash, 0, dataBuffer, ctx->attributeValueLength, NULL, 0);
        ctx->attributeListLength -= 1;

        // We have processed all attributes
        if (ctx->attributeListLength == 0) {
            uint8_t attributeHashBytes[32];
            cx_hash((cx_hash_t *) &attributeHash, CX_LAST, NULL, 0, attributeHashBytes, 32);
            toPaginatedHex(attributeHashBytes, sizeof(attributeHashBytes), ctx->attributeHashDisplay);
            ctx->state = TX_CREDENTIAL_DEPLOYMENT_LENGTH_OF_PROOFS;
            sendSuccessNoIdle();
        } else {
            // There are additional attributes to be read, so ask for more.
            ctx->state = TX_CREDENTIAL_DEPLOYMENT_ATTRIBUTE_TAG;
            sendSuccessNoIdle();
        }
    } else if (p1 == P1_LENGTH_OF_PROOFS && ctx->state == TX_CREDENTIAL_DEPLOYMENT_LENGTH_OF_PROOFS) {
        ctx->proofLength = U4BE(dataBuffer, 0);
        if (p2 == P2_CREDENTIAL_CREDENTIAL) {
            cx_hash((cx_hash_t *) &tx_state->hash, 0, dataBuffer, 4, NULL, 0);
        }
        ctx->state = TX_CREDENTIAL_DEPLOYMENT_PROOFS;
        sendSuccessNoIdle();
    } else if (p1 == P1_PROOFS && ctx->state == TX_CREDENTIAL_DEPLOYMENT_PROOFS) {
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
                    ctx->state = TX_CREDENTIAL_DEPLOYMENT_VERIFICATION_KEYS_LENGTH;
                }
            } else {
                ctx->state = TX_CREDENTIAL_DEPLOYMENT_NEW_OR_EXISTING;
            }
            sendSuccessNoIdle();
        }
    } else if (p1 == P1_NEW_OR_EXISTING && ctx->state == TX_CREDENTIAL_DEPLOYMENT_NEW_OR_EXISTING) {
        // 0 indicates new, 1 indicates existing
        uint8_t newOrExisting = dataBuffer[0];
        cx_hash((cx_hash_t *) &tx_state->hash, 0, dataBuffer, 1, NULL, 0);
        dataBuffer += 1;

        if (newOrExisting == 0) {
            cx_hash((cx_hash_t *) &tx_state->hash, 0, dataBuffer, 8, NULL, 0);
            ux_flow_init(0, ux_sign_credential_deployment_new, NULL);
        } else if (newOrExisting == 1) {
            uint8_t accountAddress[32];
            memmove(accountAddress, dataBuffer, 32);

            // Used to display account address.
            size_t outputSize = sizeof(ctx->accountAddress);
            if (base58check_encode(accountAddress, sizeof(accountAddress), ctx->accountAddress, &outputSize) != 0) {
            // The received address bytes are not a valid base58 encoding.
                THROW(ERROR_INVALID_TRANSACTION);
            }
            ctx->accountAddress[55] = '\0';

            cx_hash((cx_hash_t *) &tx_state->hash, 0, dataBuffer, 32, NULL, 0);
            ux_flow_init(0, ux_sign_credential_deployment_existing, NULL);
        } else {
            THROW(ERROR_INVALID_TRANSACTION);
        }
    } else {
        THROW(ERROR_INVALID_STATE);
    }

    *flags |= IO_ASYNCH_REPLY;
}
