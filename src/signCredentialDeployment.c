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

void parseVerificationKey(uint8_t *buffer, uint8_t dataLength) {
    // Hash key index
    if (dataLength < 1) {
        THROW(ERROR_BUFFER_OVERFLOW);  // Ensure safe access
    }
    updateHash((cx_hash_t *)&tx_state->hash, buffer, 1);
    dataLength -= 1;
    buffer += 1;

    // Hash schemeId
    updateHash((cx_hash_t *)&tx_state->hash, buffer, 1);
    if (dataLength < 1) {
        THROW(ERROR_BUFFER_OVERFLOW);  // Ensure safe access
    }
    dataLength -= 1;
    buffer += 1;

    uint8_t verificationKey[KEY_LENGTH];
    if (dataLength < KEY_LENGTH) {
        THROW(ERROR_BUFFER_OVERFLOW);  // Ensure safe access
    }
    memmove(verificationKey, buffer, KEY_LENGTH);
    updateHash((cx_hash_t *)&tx_state->hash, verificationKey, KEY_LENGTH);

    // Convert to a human-readable format.
    toPaginatedHex(verificationKey,
                   sizeof(verificationKey),
                   ctx->accountVerificationKey,
                   sizeof(ctx->accountVerificationKey));
    ctx->numberOfVerificationKeys -= 1;
}

// APDU parameters specific to credential deployment transaction (multiple packets protocol).
#define P1_INITIAL_PACKET          0x00  // Sent for 1st packet of the transfer.
#define P1_VERIFICATION_KEY_LENGTH 0x0A
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
                                uint8_t lc,
                                volatile unsigned int *flags,
                                bool isInitialCall) {
    if (isInitialCall) {
        ctx->updateCredentialState = TX_UPDATE_CREDENTIAL_INITIAL;
        ctx->state = TX_CREDENTIAL_DEPLOYMENT_VERIFICATION_KEYS_LENGTH;
    }
    uint8_t remainingDataLength = lc;
    if (p2 == P2_CREDENTIAL_INITIAL && ctx->updateCredentialState == TX_UPDATE_CREDENTIAL_INITIAL) {
        uint8_t offset = parseKeyDerivationPath(dataBuffer, remainingDataLength);
        dataBuffer += offset;
        remainingDataLength -= offset;

        if (cx_sha256_init(&tx_state->hash) != CX_SHA256) {
            THROW(ERROR_FAILED_CX_OPERATION);
        }
        offset = hashAccountTransactionHeaderAndKind(dataBuffer,
                                                     remainingDataLength,
                                                     UPDATE_CREDENTIALS);
        dataBuffer += offset;
        remainingDataLength -= offset;

        if (remainingDataLength < 1) {
            THROW(ERROR_BUFFER_OVERFLOW);
        }
        ctx->credentialDeploymentCount = dataBuffer[0];
        updateHash((cx_hash_t *)&tx_state->hash, dataBuffer, 1);
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
        if (remainingDataLength < 1) {
            THROW(ERROR_BUFFER_OVERFLOW);
        }
        updateHash((cx_hash_t *)&tx_state->hash, dataBuffer, 1);
        ctx->updateCredentialState = TX_UPDATE_CREDENTIAL_CREDENTIAL;
        sendSuccessNoIdle();
    } else if (p2 == P2_CREDENTIAL_CREDENTIAL &&
               ctx->updateCredentialState == TX_UPDATE_CREDENTIAL_CREDENTIAL &&
               ctx->credentialDeploymentCount > 0) {
        handleSignCredentialDeployment(dataBuffer, p1, p2, lc, flags, false);
    } else if (p2 == P2_CREDENTIAL_ID_COUNT &&
               ctx->updateCredentialState == TX_UPDATE_CREDENTIAL_ID_COUNT) {
        if (remainingDataLength < 1) {
            THROW(ERROR_BUFFER_OVERFLOW);
        }
        ctx->credentialIdCount = dataBuffer[0];
        updateHash((cx_hash_t *)&tx_state->hash, dataBuffer, 1);

        if (ctx->credentialIdCount == 0) {
            ctx->updateCredentialState = TX_UPDATE_CREDENTIAL_THRESHOLD;
        } else {
            ctx->updateCredentialState = TX_UPDATE_CREDENTIAL_ID;
        }
        sendSuccessNoIdle();
    } else if (p2 == P2_CREDENTIAL_ID && ctx->updateCredentialState == TX_UPDATE_CREDENTIAL_ID) {
        if (remainingDataLength < 48) {
            THROW(ERROR_BUFFER_OVERFLOW);
        }
        updateHash((cx_hash_t *)&tx_state->hash, dataBuffer, 48);
        toPaginatedHex(dataBuffer, 48, ctx->credentialId, sizeof(ctx->credentialId));

        ctx->credentialIdCount -= 1;
        if (ctx->credentialIdCount == 0) {
            ctx->updateCredentialState = TX_UPDATE_CREDENTIAL_THRESHOLD;
        }

        uiSignUpdateCredentialIdDisplay(flags);

    } else if (p2 == P2_THRESHOLD && ctx->updateCredentialState == TX_UPDATE_CREDENTIAL_THRESHOLD) {
        if (remainingDataLength < 1) {
            THROW(ERROR_BUFFER_OVERFLOW);
        }
        uint8_t threshold = dataBuffer[0];
        bin2dec(ctx->threshold, sizeof(ctx->threshold), threshold);
        updateHash((cx_hash_t *)&tx_state->hash, dataBuffer, 1);

        uiSignUpdateCredentialThresholdDisplay(flags);

    } else {
        THROW(ERROR_INVALID_STATE);
    }
}

void handleSignCredentialDeployment(uint8_t *dataBuffer,
                                    uint8_t p1,
                                    uint8_t p2,
                                    uint8_t lc,
                                    volatile unsigned int *flags,
                                    bool isInitialCall) {
    if (isInitialCall) {
        ctx->state = TX_CREDENTIAL_DEPLOYMENT_INITIAL;
    }
    uint8_t remainingDataLength = lc;

    if (p1 == P1_INITIAL_PACKET && ctx->state == TX_CREDENTIAL_DEPLOYMENT_INITIAL) {
        parseKeyDerivationPath(dataBuffer, lc);

        // Initialize values.
        if (cx_sha256_init(&tx_state->hash) != CX_SHA256) {
            THROW(ERROR_FAILED_CX_OPERATION);
        }
        ctx->state = TX_CREDENTIAL_DEPLOYMENT_VERIFICATION_KEYS_LENGTH;
        ctx->showIntro = true;

        sendSuccessNoIdle();
    } else if (p1 == P1_VERIFICATION_KEY_LENGTH &&
               ctx->state == TX_CREDENTIAL_DEPLOYMENT_VERIFICATION_KEYS_LENGTH) {
        if (lc < 1) {
            THROW(ERROR_BUFFER_OVERFLOW);
        }
        ctx->numberOfVerificationKeys = dataBuffer[0];
        updateHash((cx_hash_t *)&tx_state->hash, dataBuffer, 1);
        ctx->state = TX_CREDENTIAL_DEPLOYMENT_VERIFICATION_KEY;
        sendSuccessNoIdle();
    } else if (p1 == P1_VERIFICATION_KEY) {
        if (ctx->numberOfVerificationKeys > 0 &&
            ctx->state == TX_CREDENTIAL_DEPLOYMENT_VERIFICATION_KEY) {
            parseVerificationKey(dataBuffer, lc);
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
        if (lc < 1) {
            THROW(ERROR_BUFFER_OVERFLOW);
        }
        bin2dec(ctx->signatureThreshold, sizeof(ctx->signatureThreshold), dataBuffer[0]);
        updateHash((cx_hash_t *)&tx_state->hash, dataBuffer, 1);
        dataBuffer += 1;
        remainingDataLength -= 1;
        // Parse the RegIdCred, but do not display it, as the user cannot feasibly verify it.
        if (remainingDataLength < 48) {
            THROW(ERROR_BUFFER_OVERFLOW);
        }
        if (format_hex(dataBuffer, 48, ctx->regIdCred, sizeof(ctx->regIdCred)) == -1) {
            THROW(ERROR_BUFFER_OVERFLOW);
        }
        ctx->regIdCred[48 * 2] = '\0';
        updateHash((cx_hash_t *)&tx_state->hash, dataBuffer, 48);
        dataBuffer += 48;
        remainingDataLength -= 48;

        // Parse identity provider index.
        if (remainingDataLength < 4) {
            THROW(ERROR_BUFFER_OVERFLOW);
        }
        uint64_t identityProviderIndex = U4BE(dataBuffer, 0);
        numberToText((uint8_t *)ctx->identityProviderIndex,
                     sizeof(ctx->identityProviderIndex),
                     identityProviderIndex);
        updateHash((cx_hash_t *)&tx_state->hash, dataBuffer, 4);
        dataBuffer += 4;
        remainingDataLength -= 4;

        // Parse anonymity revocation threshold.
        if (remainingDataLength < 1) {
            THROW(ERROR_BUFFER_OVERFLOW);
        }
        int offset = numberToText(ctx->anonymityRevocationThreshold,
                                  sizeof(ctx->anonymityRevocationThreshold),
                                  dataBuffer[0]);
        if ((size_t)(offset + 8) > sizeof(ctx->anonymityRevocationThreshold)) {
            THROW(ERROR_BUFFER_OVERFLOW);
        }
        memmove(ctx->anonymityRevocationThreshold + offset, " out of ", 8);
        offset += 8;
        updateHash((cx_hash_t *)&tx_state->hash, dataBuffer, 1);
        dataBuffer += 1;
        remainingDataLength -= 1;
        // Parse the length of the following list of anonymity revokers.
        if (remainingDataLength < 2) {
            THROW(ERROR_BUFFER_OVERFLOW);
        }
        ctx->anonymityRevocationListLength = U2BE(dataBuffer, 0);
        updateHash((cx_hash_t *)&tx_state->hash, dataBuffer, 2);
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
        if (lc < 4) {
            THROW(ERROR_BUFFER_OVERFLOW);
        }
        if (format_hex(dataBuffer, 4, ctx->arIdentity, sizeof(ctx->arIdentity)) == -1) {
            THROW(ERROR_BUFFER_OVERFLOW);
        }
        ctx->arIdentity[8] = '\0';
        updateHash((cx_hash_t *)&tx_state->hash, dataBuffer, 4);
        dataBuffer += 4;
        remainingDataLength -= 4;

        // Parse enc_id_cred_pub_share
        uint8_t encIdCredPubShare[96];
        if (remainingDataLength < 96) {
            THROW(ERROR_BUFFER_OVERFLOW);
        }
        memmove(encIdCredPubShare, dataBuffer, 96);
        if (format_hex(encIdCredPubShare,
                       96,
                       ctx->encIdCredPubShare,
                       sizeof(ctx->encIdCredPubShare)) == -1) {
            THROW(ERROR_BUFFER_OVERFLOW);
        }
        ctx->encIdCredPubShare[96 * 2] = '\0';
        updateHash((cx_hash_t *)&tx_state->hash, encIdCredPubShare, 96);

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
        if (remainingDataLength < 6) {
            THROW(ERROR_BUFFER_OVERFLOW);
        }
        updateHash((cx_hash_t *)&tx_state->hash, dataBuffer, 6);
        dataBuffer += 6;
        remainingDataLength -= 6;

        // Read attribute list length
        if (remainingDataLength < 2) {
            THROW(ERROR_BUFFER_OVERFLOW);
        }
        ctx->attributeListLength = U2BE(dataBuffer, 0);
        updateHash((cx_hash_t *)&tx_state->hash, dataBuffer, 2);

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
        if (remainingDataLength < 1) {
            THROW(ERROR_BUFFER_OVERFLOW);
        }
        memmove(attributeTag, dataBuffer, 1);
        dataBuffer += 1;
        remainingDataLength -= 1;
        updateHash((cx_hash_t *)&tx_state->hash, attributeTag, 1);

        // Parse attribute length, so we know how much to parse in next packet.
        uint8_t attributeValueLength[1];
        if (remainingDataLength < 1) {
            THROW(ERROR_BUFFER_OVERFLOW);
        }
        memmove(attributeValueLength, dataBuffer, 1);
        ctx->attributeValueLength = attributeValueLength[0];
        updateHash((cx_hash_t *)&tx_state->hash, attributeValueLength, 1);

        ctx->state = TX_CREDENTIAL_DEPLOYMENT_ATTRIBUTE_VALUE;
        // Ask computer for the attribute value.
        sendSuccessNoIdle();
    } else if (p1 == P1_ATTRIBUTE_VALUE && ctx->state == TX_CREDENTIAL_DEPLOYMENT_ATTRIBUTE_VALUE) {
        // Add attribute value to the hash.
        if (remainingDataLength < ctx->attributeValueLength) {
            THROW(ERROR_BUFFER_OVERFLOW);
        }
        updateHash((cx_hash_t *)&tx_state->hash, dataBuffer, ctx->attributeValueLength);
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
        if (remainingDataLength < 4) {
            THROW(ERROR_BUFFER_OVERFLOW);
        }
        ctx->proofLength = U4BE(dataBuffer, 0);
        if (p2 == P2_CREDENTIAL_CREDENTIAL) {
            updateHash((cx_hash_t *)&tx_state->hash, dataBuffer, 4);
        }
        ctx->state = TX_CREDENTIAL_DEPLOYMENT_PROOFS;
        sendSuccessNoIdle();
    } else if (p1 == P1_PROOFS && ctx->state == TX_CREDENTIAL_DEPLOYMENT_PROOFS) {
        if (ctx->proofLength > MAX_CDATA_LENGTH) {
            if (remainingDataLength < MAX_CDATA_LENGTH) {
                THROW(ERROR_BUFFER_OVERFLOW);
            }
            updateHash((cx_hash_t *)&tx_state->hash, dataBuffer, MAX_CDATA_LENGTH);
            ctx->proofLength -= MAX_CDATA_LENGTH;
            sendSuccessNoIdle();
        } else {
            if (remainingDataLength < ctx->proofLength) {
                THROW(ERROR_BUFFER_OVERFLOW);
            }
            updateHash((cx_hash_t *)&tx_state->hash, dataBuffer, ctx->proofLength);

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
        if (remainingDataLength < 1) {
            THROW(ERROR_BUFFER_OVERFLOW);
        }
        uint8_t newOrExisting = dataBuffer[0];
        updateHash((cx_hash_t *)&tx_state->hash, dataBuffer, 1);
        dataBuffer += 1;
        remainingDataLength -= 1;
        if (newOrExisting == 0) {
            if (remainingDataLength < 8) {
                THROW(ERROR_BUFFER_OVERFLOW);
            }
            updateHash((cx_hash_t *)&tx_state->hash, dataBuffer, 8);
            if (ctx->showIntro) {
                uiSignCredentialDeploymentNewIntroDisplay();
            } else {
                uiSignCredentialDeploymentNewDisplay();
            }
        } else if (newOrExisting == 1) {
            uint8_t accountAddress[32];
            if (remainingDataLength < 32) {
                THROW(ERROR_BUFFER_OVERFLOW);
            }
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
            updateHash((cx_hash_t *)&tx_state->hash, dataBuffer, 32);

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
