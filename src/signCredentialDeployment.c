#include <os.h>
#include <os_io_seproxyhal.h>
#include "cx.h"
#include <stdint.h>
#include "menu.h"
#include "util.h"
#include <string.h>
#include "base58check.h"
#include <stdio.h>

static accountSubtreePath_t *keyPath = &path;
static signCredentialDeploymentContext_t *ctx = &global.signCredentialDeploymentContext;

// The attribute names mapping has to be consistent with ATTRIBUTE_NAMES in types.rs.
const char *ATTRIBUTE_NAMES[10] = {
    "firstName ",
    "lastName  "
};

void processNextVerificationKey();

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
// if they are invalid anyway?
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
    bn,
    sendSuccess(0),
    {
        (char *) global.signCredentialDeploymentContext.attributeTag,
        (char *) global.signCredentialDeploymentContext.attributeValue
    });
UX_FLOW(ux_credential_deployment_attributes,
    &ux_credential_deployment_attributes_0_step
);

void processNextVerificationKey() {
    if (ctx->numberOfVerificationKeys <= 0) {
        // TODO Continue to parse incoming transaction. UI idle for now to not get stuck indefinitely.
        sendSuccessNoIdle(0);
    } else {
        sendSuccessNoIdle(0);   // Request more data from the computer.
    }
}

void parseVerificationKey(uint8_t *buffer) {
    uint8_t verificationKey[32];
    os_memmove(verificationKey, buffer, 32);
    buffer += 32;

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

    ux_flow_init(0, ux_credential_deployment_initial_flow, NULL);
}

// TODO implement support for the remainder of the values.
const char* getAttributeName(uint8_t attributeTag) {
    switch (attributeTag) {
        case 1:
            return "First name";
        case 2:
            return "Last name";
        default:
            return "TODO";
    }
}

// APDU parameters specific to transfer with schedule transaction (multiple packets protocol).
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

// TODO: 'Add initialization protection to avoid concatenation of transactions.'
void handleSignCredentialDeployment(uint8_t *dataBuffer, uint8_t p1, volatile unsigned int *flags) {

    // TODO Refactor the if/else block into something nicer?


    if (p1 == P1_INITIAL_PACKET) {
        parseAccountSignatureKeyPath(dataBuffer);
        dataBuffer += 2;

        os_memmove(ctx->displayAccount, "with #", 6);
        bin2dec(ctx->displayAccount + 6, keyPath->accountIndex);

        parseVerificationKeysLength(dataBuffer);
    } else if (p1 == P1_VERIFICATION_KEY) {
        if (ctx->numberOfVerificationKeys > 0) {
            parseVerificationKey(dataBuffer);
        } else {
            THROW(0x6B01);  // Invalid state, sender says a verification is incoming, but we already received all.
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

        // Parse RegIdCred and make it displayable as hex.
        uint8_t regIdCred[48];
        os_memmove(regIdCred, dataBuffer, 48);
        dataBuffer += 48;
        toHex(regIdCred, sizeof(regIdCred), ctx->regIdCred);

        // Parse identity provider identity.
        uint8_t identityProviderIdentity[4];
        os_memmove(identityProviderIdentity, dataBuffer, sizeof(identityProviderIdentity));
        uint32_t identityProviderValue = U4BE(identityProviderIdentity, 0);
        bin2dec(ctx->identityProviderIdentity, identityProviderValue);
        dataBuffer += 4;

        // Parse anonymity revocation threshold.
        os_memmove(temp, dataBuffer, 1);
        bin2dec(ctx->anonymityRevocationThreshold, temp[0]);
        dataBuffer += 1;

        // Parse the length of the following list of anonymity revokers.
        ctx->anonymityRevocationListLength = U2BE(dataBuffer, 0);
        dataBuffer += 2;

        // TODO Update state that the signature threshold step has completed.

        // Display the loaded data.
        ux_flow_init(0, ux_credential_deployment_threshold_flow, NULL);
    } else if (p1 == P1_AR_IDENTITY) {

        // TODO Fail if invalid state at this point.

        if (ctx->anonymityRevocationListLength <= 0) {
            THROW(0x6B01);  // Invalid state, sender says ar identity pair is incoming, but we already received all.
        }

        // Parse ArIdentity
        uint32_t arIdentity = U4BE(dataBuffer, 0);
        bin2dec(ctx->arIdentity, arIdentity);
        dataBuffer += 4;

        // Parse enc_id_cred_pub_share
        uint8_t encIdCredPubShare[96];
        os_memmove(encIdCredPubShare, dataBuffer, 96);
        toHex(encIdCredPubShare, sizeof(encIdCredPubShare), ctx->encIdCredPubShare);
        dataBuffer += 96;

        // Display the loaded data.
        ux_flow_init(0, ux_credential_deployment_aridentity_key_flow, NULL);
    } else if (p1 == P1_CREDENTIAL_DATES) {
        // TODO Consider validating that the values are sensible.

        uint8_t temp[1];

        // Build display of valid to
        uint16_t validToYear = U2BE(dataBuffer, 0);
        dataBuffer += 2;
        os_memmove(temp, dataBuffer, 1);
        dataBuffer += 1;
        bin2dec(ctx->validTo, validToYear);
        ctx->validTo[4] = ' ';
        bin2dec(ctx->validTo + 5, temp[0]);

        // Build display of created at
        uint16_t createdAtYear = U2BE(dataBuffer, 0);
        dataBuffer += 2;
        os_memmove(temp, dataBuffer, 1);
        dataBuffer += 1;
        bin2dec(ctx->createdAt, createdAtYear);
        ctx->createdAt[4] = ' ';
        bin2dec(ctx->createdAt + 5, temp[0]);

        // Read attribute list length
        ctx->attributeListLength = U2BE(dataBuffer, 0);

        // TODO Update state

        ux_flow_init(0, ux_credential_deployment_dates, NULL);
    } else if (p1 == P1_ATTRIBUTE_TAG) {
        // TODO Validate state, i.e. that we have seen credential states.

        // Parse attribute tag, and map it the attribute name (the display text).
        uint8_t attributeTag[1];
        os_memmove(attributeTag, dataBuffer, 1);
        os_memmove(ctx->attributeTag, getAttributeName(attributeTag[0]), 10);
        dataBuffer += 1;

        // Parse attribute length, so we know how much to parse in next packet.
        uint8_t attributeValueLength[1];
        os_memmove(attributeValueLength, dataBuffer, 1);
        ctx->attributeValueLength = attributeValueLength[0];

        // Ask computer for the attribute value.
        sendSuccessNoIdle(0);
    } else if (p1 == P1_ATTRIBUTE_VALUE) {
        // TODO Check for state and all that jazz.

        // TODO Attribute values are encoded with UTF-8... To display anything sensible we would need a UTF-8
        // TODO decoder to display it.
        // Parse attribute value and display it.

        os_memmove(ctx->attributeValue, dataBuffer, ctx->attributeValueLength);
        ctx->attributeValue[255] = '\0';

        ux_flow_init(0, ux_credential_deployment_attributes, NULL);
    }

    *flags |= IO_ASYNCH_REPLY;
}















