#include "globals.h"

static signCredentialDeploymentContext_t *ctx = &global.signCredentialDeploymentContext;
static tx_state_t *tx_state = &global_tx_state;

void processNextVerificationKey(void) {
    if (ctx->numberOfVerificationKeys == 0) {
        ctx->state = TX_CREDENTIAL_DEPLOYMENT_SIGNATURE_THRESHOLD;
        sendSuccessNoIdle();
    } else {
        sendSuccessNoIdle();  // Request more data from the computer.
    }
}

void parseVerificationKey(uint8_t *buffer) {
    // Hash key index
    updateHash((cx_hash_t *) &tx_state->hash, buffer, 1);
    buffer += 1;

    // Hash schemeId
    updateHash((cx_hash_t *) &tx_state->hash, buffer, 1);
    buffer += 1;

    uint8_t verificationKey[32];
    memmove(verificationKey, buffer, 32);
    updateHash((cx_hash_t *) &tx_state->hash, verificationKey, 32);

    // Convert to a human-readable format.
    toPaginatedHex(verificationKey,
                   sizeof(verificationKey),
                   ctx->accountVerificationKey,
                   sizeof(ctx->accountVerificationKey));
    ctx->numberOfVerificationKeys -= 1;
}

// APDU parameters specific to credential deployment transaction (multiple packets protocol).
#define P1_INITIAL_PACKET          0x00  // Sent for 1st packet of the transfer.
#define P1_VERIFICATION_KEY_LENGTH 0x0A  // TODO: Move to 0x02
#define P1_VERIFICATION_KEY        0x01  // Sent for packets containing a verification key.
#define P1_SIGNATURE_THRESHOLD \
    0x02  // Sent for the packet containing signature threshold, RegIdCred,
          // identity provider identity, anonymity invocation threshold
          // and the length of the anonymity revocation data.
#define P1_AR_IDENTITY \
    0x03  // Sent for the packets containing a aridentity / encidcredpubshares pair.
#define P1_CREDENTIAL_DATES \
    0x04  // Sent for the packet containing the credential valid to / create at dates.
#define P1_ATTRIBUTE_TAG \
    0x05  // Sent for the packet containing the attribute tag, and the attribute
          // value length, which is used to read the attribute value.
#define P1_ATTRIBUTE_VALUE  0x06  // Sent for the packet containing an attribute value.
#define P1_LENGTH_OF_PROOFS 0x07  // Sent for the packet containing the byte length of the proofs.
#define P1_PROOFS           0x08  // Sent for the packets containing proof bytes.
#define P1_NEW_OR_EXISTING  0x09

#define P2_CREDENTIAL_INITIAL          0x00
#define P2_CREDENTIAL_CREDENTIAL_INDEX 0x01
#define P2_CREDENTIAL_CREDENTIAL       0x02
#define P2_CREDENTIAL_ID_COUNT         0x03
#define P2_CREDENTIAL_ID               0x04
#define P2_THRESHOLD                   0x05

void handleSignUpdateCredential(uint8_t *dataBuffer,
                                uint8_t p1,
                                uint8_t p2,
                                volatile unsigned int *flags,
                                bool isInitialCall) {
    if (isInitialCall) {
        ctx->updateCredentialState = TX_UPDATE_CREDENTIAL_INITIAL;
        ctx->state = TX_CREDENTIAL_DEPLOYMENT_VERIFICATION_KEYS_LENGTH;
    }

    if (p2 == P2_CREDENTIAL_INITIAL && ctx->updateCredentialState == TX_UPDATE_CREDENTIAL_INITIAL) {
        dataBuffer += parseKeyDerivationPath(dataBuffer);

        cx_sha256_init(&tx_state->hash);
        dataBuffer += hashAccountTransactionHeaderAndKind(dataBuffer, UPDATE_CREDENTIALS);

        ctx->credentialDeploymentCount = dataBuffer[0];
        updateHash((cx_hash_t *) &tx_state->hash, dataBuffer, 1);
        if (ctx->credentialDeploymentCount == 0) {
            ctx->updateCredentialState = TX_UPDATE_CREDENTIAL_ID_COUNT;
        } else {
            ctx->updateCredentialState = TX_UPDATE_CREDENTIAL_CREDENTIAL_INDEX;
        }

        uiSignUpdateCredentialInitialDisplay(flags);

    } else if (p2 == P2_CREDENTIAL_CREDENTIAL_INDEX &&
               ctx->updateCredentialState == TX_UPDATE_CREDENTIAL_CREDENTIAL_INDEX &&
               ctx->credentialDeploymentCount > 0) {
        // Add the credential index to the hash
        updateHash((cx_hash_t *) &tx_state->hash, dataBuffer, 1);
        ctx->updateCredentialState = TX_UPDATE_CREDENTIAL_CREDENTIAL;
        sendSuccessNoIdle();
    } else if (p2 == P2_CREDENTIAL_CREDENTIAL &&
               ctx->updateCredentialState == TX_UPDATE_CREDENTIAL_CREDENTIAL &&
               ctx->credentialDeploymentCount > 0) {
        handleSignCredentialDeployment(dataBuffer, p1, p2, flags, false);
    } else if (p2 == P2_CREDENTIAL_ID_COUNT &&
               ctx->updateCredentialState == TX_UPDATE_CREDENTIAL_ID_COUNT) {
        ctx->credentialIdCount = dataBuffer[0];
        updateHash((cx_hash_t *) &tx_state->hash, dataBuffer, 1);

        if (ctx->credentialIdCount == 0) {
            ctx->updateCredentialState = TX_UPDATE_CREDENTIAL_THRESHOLD;
        } else {
            ctx->updateCredentialState = TX_UPDATE_CREDENTIAL_ID;
        }
        sendSuccessNoIdle();
    } else if (p2 == P2_CREDENTIAL_ID && ctx->updateCredentialState == TX_UPDATE_CREDENTIAL_ID) {
        updateHash((cx_hash_t *) &tx_state->hash, dataBuffer, 48);
        toPaginatedHex(dataBuffer, 48, ctx->credentialId, sizeof(ctx->credentialId));

        ctx->credentialIdCount -= 1;
        if (ctx->credentialIdCount == 0) {
            ctx->updateCredentialState = TX_UPDATE_CREDENTIAL_THRESHOLD;
        }

        uiSignUpdateCredentialIdDisplay(flags);

    } else if (p2 == P2_THRESHOLD && ctx->updateCredentialState == TX_UPDATE_CREDENTIAL_THRESHOLD) {
        uint8_t threshold = dataBuffer[0];
        bin2dec(ctx->threshold, sizeof(ctx->threshold), threshold);
        updateHash((cx_hash_t *) &tx_state->hash, dataBuffer, 1);

        uiSignUpdateCredentialThresholdDisplay(flags);

    } else {
        THROW(ERROR_INVALID_STATE);
    }
}

void handleSignCredentialDeployment(uint8_t *dataBuffer,
                                    uint8_t p1,
                                    uint8_t p2,
                                    volatile unsigned int *flags,
                                    bool isInitialCall) {
    if (isInitialCall) {
        ctx->state = TX_CREDENTIAL_DEPLOYMENT_INITIAL;
    }

    if (p1 == P1_INITIAL_PACKET && ctx->state == TX_CREDENTIAL_DEPLOYMENT_INITIAL) {
        parseKeyDerivationPath(dataBuffer);

        // Initialize values.
        cx_sha256_init(&tx_state->hash);
        ctx->state = TX_CREDENTIAL_DEPLOYMENT_VERIFICATION_KEYS_LENGTH;
        ctx->showIntro = true;

        sendSuccessNoIdle();
    } else if (p1 == P1_VERIFICATION_KEY_LENGTH &&
               ctx->state == TX_CREDENTIAL_DEPLOYMENT_VERIFICATION_KEYS_LENGTH) {
        ctx->numberOfVerificationKeys = dataBuffer[0];
        updateHash((cx_hash_t *) &tx_state->hash, dataBuffer, 1);
        ctx->state = TX_CREDENTIAL_DEPLOYMENT_VERIFICATION_KEY;
        sendSuccessNoIdle();
    } else if (p1 == P1_VERIFICATION_KEY) {
        if (ctx->numberOfVerificationKeys > 0 &&
            ctx->state == TX_CREDENTIAL_DEPLOYMENT_VERIFICATION_KEY) {
            parseVerificationKey(dataBuffer);
        } else {
            THROW(ERROR_INVALID_STATE);
        }

        if (ctx->numberOfVerificationKeys > 0) {
            if (ctx->showIntro) {
                // For the first key we also display the initial view.
                ctx->showIntro = false;
                uiSignCredentialDeploymentVerificationKeyDisplay(flags);

            } else {
                // Display a key with continue here.
                uiSignCredentialDeploymentVerificationKeyFlowDisplay(flags);
            }
        } else {
            // Do not display the last verification key here. This is deferred to the final UI flow.
            ctx->state = TX_CREDENTIAL_DEPLOYMENT_SIGNATURE_THRESHOLD;
            sendSuccessNoIdle();
        }

    } else if (p1 == P1_SIGNATURE_THRESHOLD &&
               ctx->state == TX_CREDENTIAL_DEPLOYMENT_SIGNATURE_THRESHOLD) {
        if (ctx->numberOfVerificationKeys != 0) {
            THROW(ERROR_INVALID_STATE);  // Invalid state, the sender has not sent all verification
                                         // keys before moving on.
        }

        // Parse signature threshold.
        bin2dec(ctx->signatureThreshold, sizeof(ctx->signatureThreshold), dataBuffer[0]);
        updateHash((cx_hash_t *) &tx_state->hash, dataBuffer, 1);
        dataBuffer += 1;

        // Parse the RegIdCred, but do not display it, as the user cannot feasibly verify it.
        updateHash((cx_hash_t *) &tx_state->hash, dataBuffer, 48);
        dataBuffer += 48;

        // Parse identity provider index.
        // We do not show the identity provider id, because it is infeasible for the user to
        // validate it, and there are no known reasonable attacks made possible by replacing this.
        updateHash((cx_hash_t *) &tx_state->hash, dataBuffer, 4);
        dataBuffer += 4;

        // Parse anonymity revocation threshold.
        int offset = numberToText(ctx->anonymityRevocationThreshold,
                                  sizeof(ctx->anonymityRevocationThreshold),
                                  dataBuffer[0]);
        memmove(ctx->anonymityRevocationThreshold + offset, " out of ", 8);
        offset += 8;
        updateHash((cx_hash_t *) &tx_state->hash, dataBuffer, 1);
        dataBuffer += 1;

        // Parse the length of the following list of anonymity revokers.
        ctx->anonymityRevocationListLength = U2BE(dataBuffer, 0);
        updateHash((cx_hash_t *) &tx_state->hash, dataBuffer, 2);
        // Add the total amount of revokers to the display of threshold to get "x out of y"
        bin2dec(ctx->anonymityRevocationThreshold + offset,
                sizeof(ctx->anonymityRevocationThreshold) - offset,
                ctx->anonymityRevocationListLength);

        ctx->state = TX_CREDENTIAL_DEPLOYMENT_AR_IDENTITY;

        sendSuccessNoIdle();
    } else if (p1 == P1_AR_IDENTITY && ctx->state == TX_CREDENTIAL_DEPLOYMENT_AR_IDENTITY) {
        if (ctx->anonymityRevocationListLength == 0) {
            // Invalid state, sender says ar identity pair is incoming, but we already received all.
            THROW(ERROR_INVALID_STATE);
        }

        // Parse ArIdentity
        // We do not show the AR identity id, because it is infeasible for the user to validate it,
        // and there are no known reasonable attacks made possible by replacing this.
        updateHash((cx_hash_t *) &tx_state->hash, dataBuffer, 4);
        dataBuffer += 4;

        // Parse enc_id_cred_pub_share
        // We do not show encrypted shares, as they are not possible for a user
        // to validate.
        uint8_t encIdCredPubShare[96];
        memmove(encIdCredPubShare, dataBuffer, 96);
        updateHash((cx_hash_t *) &tx_state->hash, encIdCredPubShare, 96);

        if (ctx->anonymityRevocationListLength == 1) {
            ctx->state = TX_CREDENTIAL_DEPLOYMENT_CREDENTIAL_DATES;
        }
        ctx->anonymityRevocationListLength -= 1;
        sendSuccessNoIdle();
    } else if (p1 == P1_CREDENTIAL_DATES &&
               ctx->state == TX_CREDENTIAL_DEPLOYMENT_CREDENTIAL_DATES) {
        // hash valid to and created at
        // We don't show these values, because only the dates on the identity object can be accepted
        // by the chain.
        updateHash((cx_hash_t *) &tx_state->hash, dataBuffer, 6);
        dataBuffer += 6;

        // Read attribute list length
        ctx->attributeListLength = U2BE(dataBuffer, 0);
        updateHash((cx_hash_t *) &tx_state->hash, dataBuffer, 2);

        if (ctx->attributeListLength == 0) {
            ctx->state = TX_CREDENTIAL_DEPLOYMENT_LENGTH_OF_PROOFS;
        } else {
            ctx->state = TX_CREDENTIAL_DEPLOYMENT_ATTRIBUTE_TAG;
        }

        sendSuccessNoIdle();
    } else if (p1 == P1_ATTRIBUTE_TAG && ctx->state == TX_CREDENTIAL_DEPLOYMENT_ATTRIBUTE_TAG) {
        if (ctx->attributeListLength <= 0) {
            THROW(ERROR_INVALID_STATE);
        }

        // Parse attribute tag, and map it the attribute name (the display text).
        uint8_t attributeTag[1];
        memmove(attributeTag, dataBuffer, 1);
        dataBuffer += 1;
        updateHash((cx_hash_t *) &tx_state->hash, attributeTag, 1);

        // Parse attribute length, so we know how much to parse in next packet.
        uint8_t attributeValueLength[1];
        memmove(attributeValueLength, dataBuffer, 1);
        ctx->attributeValueLength = attributeValueLength[0];
        updateHash((cx_hash_t *) &tx_state->hash, attributeValueLength, 1);

        ctx->state = TX_CREDENTIAL_DEPLOYMENT_ATTRIBUTE_VALUE;
        // Ask computer for the attribute value.
        sendSuccessNoIdle();
    } else if (p1 == P1_ATTRIBUTE_VALUE && ctx->state == TX_CREDENTIAL_DEPLOYMENT_ATTRIBUTE_VALUE) {
        // Add attribute value to the hash.
        updateHash((cx_hash_t *) &tx_state->hash, dataBuffer, ctx->attributeValueLength);
        ctx->attributeListLength -= 1;

        // We have processed all attributes
        if (ctx->attributeListLength == 0) {
            ctx->state = TX_CREDENTIAL_DEPLOYMENT_LENGTH_OF_PROOFS;
            sendSuccessNoIdle();
        } else {
            // There are additional attributes to be read, so ask for more.
            ctx->state = TX_CREDENTIAL_DEPLOYMENT_ATTRIBUTE_TAG;
            sendSuccessNoIdle();
        }
    } else if (p1 == P1_LENGTH_OF_PROOFS &&
               ctx->state == TX_CREDENTIAL_DEPLOYMENT_LENGTH_OF_PROOFS) {
        ctx->proofLength = U4BE(dataBuffer, 0);
        if (p2 == P2_CREDENTIAL_CREDENTIAL) {
            updateHash((cx_hash_t *) &tx_state->hash, dataBuffer, 4);
        }
        ctx->state = TX_CREDENTIAL_DEPLOYMENT_PROOFS;
        sendSuccessNoIdle();
    } else if (p1 == P1_PROOFS && ctx->state == TX_CREDENTIAL_DEPLOYMENT_PROOFS) {
        if (ctx->proofLength > MAX_CDATA_LENGTH) {
            updateHash((cx_hash_t *) &tx_state->hash, dataBuffer, MAX_CDATA_LENGTH);
            ctx->proofLength -= MAX_CDATA_LENGTH;
            sendSuccessNoIdle();
        } else {
            updateHash((cx_hash_t *) &tx_state->hash, dataBuffer, ctx->proofLength);

            // If an update credential transaction, then update state to next step.
            if (p2 == P2_CREDENTIAL_CREDENTIAL &&
                ctx->updateCredentialState == TX_UPDATE_CREDENTIAL_CREDENTIAL &&
                ctx->credentialDeploymentCount > 0) {
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
        updateHash((cx_hash_t *) &tx_state->hash, dataBuffer, 1);
        dataBuffer += 1;

        if (newOrExisting == 0) {
            updateHash((cx_hash_t *) &tx_state->hash, dataBuffer, 8);
            if (ctx->showIntro) {
                uiSignCredentialDeploymentNewIntroDisplay();
            } else {
                uiSignCredentialDeploymentNewDisplay();
            }
        } else if (newOrExisting == 1) {
            uint8_t accountAddress[32];
            memmove(accountAddress, dataBuffer, 32);

            // Used to display account address.
            size_t outputSize = sizeof(ctx->accountAddress);
            if (base58check_encode(accountAddress,
                                   sizeof(accountAddress),
                                   ctx->accountAddress,
                                   &outputSize) == -1) {
                // The received address bytes are not a valid base58 encoding.
                THROW(ERROR_INVALID_TRANSACTION);
            }
            ctx->accountAddress[55] = '\0';
            updateHash((cx_hash_t *) &tx_state->hash, dataBuffer, 32);

            if (ctx->showIntro) {
                uiSignCredentialDeploymentExistingIntroDisplay();
            } else {
                uiSignCredentialDeploymentExistingDisplay();
            }
        } else {
            THROW(ERROR_INVALID_TRANSACTION);
        }
    } else {
        THROW(ERROR_INVALID_STATE);
    }

    *flags |= IO_ASYNCH_REPLY;
}
