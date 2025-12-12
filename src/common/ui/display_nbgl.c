#ifdef HAVE_NBGL
#include "globals.h"
#include "display.h"

accountSender_t global_account_sender;
static nbgl_contentTagValue_t pairs[32];  // Increased to handle multiple PLT operations
static signTransferWithScheduleContext_t *ctx_sign_transfer_with_schedule =
    &global.withDataBlob.signTransferWithScheduleContext;

static void review_choice(bool confirm) {
    // Answer, display a status page and go back to main
    if (confirm) {
        nbgl_useCaseReviewStatus(STATUS_TYPE_ADDRESS_VERIFIED, ui_menu_main);
    } else {
        nbgl_useCaseReviewStatus(STATUS_TYPE_ADDRESS_REJECTED, ui_menu_main);
    }
}

static void review_public_key_choice(bool confirm) {
    // Answer, display a status page and go back to main
    if (confirm) {
        sendPublicKey(true);
    } else {
        sendUserRejection();
    }
}

static void review_verify_address(bool confirm) {
    if (confirm) {
        sendSuccessResultNoIdle(0);
        nbgl_useCaseReviewStatus(STATUS_TYPE_ADDRESS_VERIFIED, ui_menu_main);
    } else {
        sendUserRejectionNoIdle();
        nbgl_useCaseReviewStatus(STATUS_TYPE_ADDRESS_REJECTED, ui_menu_main);
    }
}

static void review_export_private_key(bool confirm) {
    // Answer, display a status page and go back to main
    if (confirm) {
        exportPrivateKey();
    } else {
        sendUserRejection();
    }
}

static void review_export_private_key_new_path(bool confirm) {
    // Answer, display a status page and go back to main
    if (confirm) {
        sendPrivateKeysNewPath();
    } else {
        sendUserRejection();
    }
}

static void review_choice_sign(bool confirm) {
    // Answer, display a status page and go back to main
    if (confirm) {
        buildAndSignTransactionHash();
    } else {
        sendUserRejection();
    }
}

static void keep_going_with_transaction(bool confirm) {
    if (confirm) {
        sendSuccessNoIdle();
    } else {
        sendUserRejection();
    }
}
static void sendSuccessNoIdleCallback(bool confirm) {
    (void)confirm;  // Suppress unused parameter warning
    sendSuccessNoIdle();
}

static void processNextVerificationKeyNoIdleCallback(bool confirm) {
    (void)confirm;  // Suppress unused parameter warning
    processNextVerificationKey();
}

void uiComparePubkey(void) {
    nbgl_useCaseAddressReview(global.exportPublicKeyContext.publicKey,
                              NULL,
                              &ICON_APP_HOME,
                              "Compare",
                              NULL,
                              review_choice);
}

void uiGeneratePubkey(volatile unsigned int *flags) {
    nbgl_useCaseAddressReview((char *)global.exportPublicKeyContext.display,  // Address to display
                              NULL,                     // No additional tag-value list
                              &ICON_APP_HOME,           // Icon to display
                              "Public Key",             // Review title
                              NULL,                     // No review subtitle
                              review_public_key_choice  // Callback function
    );
    *flags |= IO_ASYNCH_REPLY;
}

void uiExportPrivateKey(volatile unsigned int *flags) {
    // Create tag-value pairs for the content
    uint8_t pairIndex = 0;
    pairs[pairIndex].item = (char *)global.exportPrivateKeyContext.displayHeader;
    pairs[pairIndex].value = (char *)global.exportPrivateKeyContext.display;
    pairIndex++;

    // Create the page content
    nbgl_contentTagValueList_t content;
    content.nbPairs = pairIndex;
    content.pairs = pairs;
    content.smallCaseForValue = false;
    content.nbMaxLinesForValue = 0;
    content.startIndex = 0;

    // Setup the review screen
    nbgl_useCaseReview(TYPE_OPERATION,
                       &content,
                       &ICON_APP_HOME,
                       "Export Private Key",
                       NULL,
                       "Accept",
                       review_export_private_key);
    *flags |= IO_ASYNCH_REPLY;
}

void uiExportPrivateKeysNewPath(volatile unsigned int *flags) {
    // Create tag-value pairs for the content
    uint8_t pairIndex = 0;
    pairs[pairIndex].item = (char *)global.exportPrivateKeyContext.displayHeader;
    pairs[pairIndex].value = (char *)global.exportPrivateKeyContext.display;
    pairIndex++;

    // Create the page content
    nbgl_contentTagValueList_t content;
    content.nbPairs = pairIndex;
    content.pairs = pairs;
    content.smallCaseForValue = false;
    content.nbMaxLinesForValue = 0;
    content.startIndex = 0;

    // Setup the review screen
    nbgl_useCaseReview(TYPE_OPERATION,
                       &content,
                       &C_app_concordium_64px,
                       "Export Private Keys for",
                       NULL,
                       "Accept",
                       review_export_private_key_new_path);
    *flags |= IO_ASYNCH_REPLY;
}

void startConfigureBakerCommissionDisplay(void) {
    // Get context from global state
    signConfigureBaker_t *ctx = &global.signConfigureBaker;
    // Create tag-value pairs for the content
    uint8_t pairIndex = 0;

    if (ctx->firstDisplay) {
        // Add sender address
        pairs[pairIndex].item = "Sender";
        pairs[pairIndex].value = (char *)global_account_sender.sender;
        pairIndex++;
        ctx->firstDisplay = false;
    }

    if (ctx->hasTransactionFeeCommission || ctx->hasBakingRewardCommission ||
        ctx->hasFinalizationRewardCommission) {
        pairs[pairIndex].item = "Commission";
        pairs[pairIndex].value = "rates";
        pairIndex++;
    }

    if (ctx->hasTransactionFeeCommission) {
        pairs[pairIndex].item = "Transaction fee";
        pairs[pairIndex].value =
            (char *)global.signConfigureBaker.commissionRates.transactionFeeCommissionRate;
        pairIndex++;
    }

    if (ctx->hasBakingRewardCommission) {
        pairs[pairIndex].item = "Baking reward";
        pairs[pairIndex].value =
            (char *)global.signConfigureBaker.commissionRates.bakingRewardCommissionRate;
        pairIndex++;
    }

    if (ctx->hasFinalizationRewardCommission) {
        pairs[pairIndex].item = "Finalization reward";
        pairs[pairIndex].value =
            (char *)global.signConfigureBaker.commissionRates.finalizationRewardCommissionRate;
        pairIndex++;
    }

    // Create the page content
    nbgl_contentTagValueList_t content;
    content.nbPairs = pairIndex;
    content.pairs = pairs;
    content.smallCaseForValue = false;
    content.nbMaxLinesForValue = 0;
    content.startIndex = 0;

    // to signing screens.
    if (ctx->hasSuspended) {
        // Setup the review screen
        nbgl_useCaseReviewLight(TYPE_OPERATION,
                                &content,
                                &ICON_APP_HOME,
                                "Review Transaction",
                                NULL,  // No subtitle
                                "Continue with transaction",
                                sendSuccessNoIdleCallback);
    } else {
        // Setup the review screen
        nbgl_useCaseReview(TYPE_TRANSACTION,
                           &content,
                           &ICON_APP_HOME,
                           "Review Transaction",
                           NULL,  // No subtitle
                           "Sign transaction",
                           review_choice_sign);
    }
}

void startConfigureBakerSuspendedDisplay(void) {
    // Get context from global state
    signConfigureBaker_t *ctx = &global.signConfigureBaker;
    // Create tag-value pairs for the content
    uint8_t pairIndex = 0;

    if (ctx->firstDisplay) {
        // Add sender address
        pairs[pairIndex].item = "Sender";
        pairs[pairIndex].value = (char *)global_account_sender.sender;
        pairIndex++;
        ctx->firstDisplay = false;
    }

    pairs[pairIndex].item = "Validator status";
    pairs[pairIndex].value = (char *)global.signConfigureBaker.suspended;
    pairIndex++;

    // Create the page content
    nbgl_contentTagValueList_t content;
    content.nbPairs = pairIndex;
    content.pairs = pairs;
    content.smallCaseForValue = false;
    content.nbMaxLinesForValue = 0;
    content.startIndex = 0;

    // to signing screens.
    nbgl_useCaseReview(TYPE_TRANSACTION,
                       &content,
                       &ICON_APP_HOME,
                       "Review Transaction",
                       NULL,  // No subtitle
                       "Sign transaction",
                       review_choice_sign);
}

void startConfigureBakerDisplay(void) {
    // Get context from global state
    signConfigureBaker_t *ctx = &global.signConfigureBaker;

    // Create tag-value pairs for the content
    uint8_t pairIndex = 0;
    // Add sender address
    pairs[pairIndex].item = "Sender";
    pairs[pairIndex].value = (char *)global_account_sender.sender;
    pairIndex++;

    ctx->firstDisplay = false;

    if (ctx->hasCapital) {
        if (ctx->capitalRestakeDelegation.stopBaking) {
            pairs[pairIndex].item = "Stop";
            pairs[pairIndex].value = "Baking";
        } else {
            pairs[pairIndex].item = "Amount to stake";
            pairs[pairIndex].value =
                (char *)global.signConfigureBaker.capitalRestakeDelegation.displayCapital;
        }
        pairIndex++;
    }

    if (ctx->hasRestakeEarnings) {
        pairs[pairIndex].item = "Restake earnings";
        pairs[pairIndex].value =
            (char *)global.signConfigureBaker.capitalRestakeDelegation.displayRestake;
        pairIndex++;
    }

    if (ctx->hasOpenForDelegation) {
        pairs[pairIndex].item = "Pool status";
        pairs[pairIndex].value =
            (char *)global.signConfigureBaker.capitalRestakeDelegation.displayOpenForDelegation;
        pairIndex++;
    }

    if (ctx->hasKeys) {
        pairs[pairIndex].item = "Update baker";
        pairs[pairIndex].value = "keys";
        pairIndex++;
    }

    // If there are additional steps, then show continue screen. If this is the last step,
    // then show signing screens.
    if (ctx->hasMetadataUrl || hasCommissionRate()) {
        // Create the page content
        nbgl_contentTagValueList_t content;
        content.nbPairs = pairIndex;
        content.pairs = pairs;
        content.smallCaseForValue = false;
        content.nbMaxLinesForValue = 0;
        content.startIndex = 0;
        // Setup the review screen
        nbgl_useCaseReviewLight(TYPE_OPERATION,
                                &content,
                                &ICON_APP_HOME,
                                "Review Transaction",
                                NULL,  // No subtitle
                                "Continue with transaction",
                                sendSuccessNoIdleCallback);
    } else {
        // Create the page content
        nbgl_contentTagValueList_t content;
        content.nbPairs = pairIndex;
        content.pairs = pairs;
        content.smallCaseForValue = false;
        content.nbMaxLinesForValue = 0;
        content.startIndex = 0;

        // Setup the review screen
        nbgl_useCaseReview(TYPE_TRANSACTION,
                           &content,
                           &ICON_APP_HOME,
                           "Review Transaction",
                           NULL,  // No subtitle
                           "Sign transaction",
                           review_choice_sign);
    }
}

void startConfigureBakerUrlDisplay(bool lastUrlPage) {
    // Get context from global state
    signConfigureBaker_t *ctx = &global.signConfigureBaker;

    // Create tag-value pairs for the content
    uint8_t pairIndex = 0;

    if (ctx->firstDisplay) {
        // Add sender address
        pairs[pairIndex].item = "Sender";
        pairs[pairIndex].value = (char *)global_account_sender.sender;
        pairIndex++;
        ctx->firstDisplay = false;
    }

    if (!lastUrlPage) {
        pairs[pairIndex].item = "URL";
        pairs[pairIndex].value = (char *)global.signConfigureBaker.url.urlDisplay;
        pairIndex++;
    } else {
        if (ctx->url.urlLength == 0) {
            pairs[pairIndex].item = "Empty URL";
            pairs[pairIndex].value = "";
        } else {
            pairs[pairIndex].item = "URL";
            pairs[pairIndex].value = (char *)global.signConfigureBaker.url.urlDisplay;
        }
        pairIndex++;
    }

    // If there are additional steps show the continue screen, otherwise go
    // to signing screens.
    if (hasCommissionRate()) {
        // Create the page content
        nbgl_contentTagValueList_t content;
        content.nbPairs = pairIndex;
        content.pairs = pairs;
        content.smallCaseForValue = false;
        content.nbMaxLinesForValue = 0;
        content.startIndex = 0;
        // Setup the review screen
        nbgl_useCaseReviewLight(TYPE_OPERATION,
                                &content,
                                &ICON_APP_HOME,
                                "Review Transaction",
                                NULL,  // No subtitle
                                "Continue with transaction",
                                sendSuccessNoIdleCallback);
    } else {
        // Create the page content
        nbgl_contentTagValueList_t content;
        content.nbPairs = pairIndex;
        content.pairs = pairs;
        content.smallCaseForValue = false;
        content.nbMaxLinesForValue = 0;
        content.startIndex = 0;

        // Setup the review screen
        nbgl_useCaseReview(TYPE_TRANSACTION,
                           &content,
                           &ICON_APP_HOME,
                           "Review Transaction",
                           NULL,  // No subtitle
                           "Sign transaction",
                           review_choice_sign);
    }
}

void startConfigureDelegationDisplay(void) {
    // Get context from global state
    signConfigureDelegationContext_t *ctx = &global.signConfigureDelegation;

    // Create tag-value pairs for the content
    uint8_t pairIndex = 0;
    // Add sender address
    pairs[pairIndex].item = "Sender";
    pairs[pairIndex].value = (char *)global_account_sender.sender;
    pairIndex++;

    // Add capital amount if present
    if (ctx->hasCapital) {
        if (ctx->stopDelegation) {
            pairs[pairIndex].item = "Action";
            pairs[pairIndex].value = "Stop delegation";
        } else {
            pairs[pairIndex].item = "Amount to delegate";
            pairs[pairIndex].value = (char *)ctx->displayCapital;
        }
        pairIndex++;
    }

    // Add restake earnings if present
    if (ctx->hasRestakeEarnings) {
        pairs[pairIndex].item = "Restake earnings";
        pairs[pairIndex].value = (char *)ctx->displayRestake;
        pairIndex++;
    }

    // Add delegation target if present
    if (ctx->hasDelegationTarget) {
        pairs[pairIndex].item = "Delegation target";
        pairs[pairIndex].value = (char *)ctx->displayDelegationTarget;
        pairIndex++;
    }

    // Create the page content
    nbgl_contentTagValueList_t content;
    content.nbPairs = pairIndex;
    content.pairs = pairs;
    content.smallCaseForValue = false;
    content.nbMaxLinesForValue = 0;
    content.startIndex = 0;

    // Setup the review screen
    nbgl_useCaseReview(TYPE_TRANSACTION,
                       &content,
                       &ICON_APP_HOME,
                       "Review Transaction",
                       NULL,  // No subtitle
                       "Sign transaction",
                       review_choice_sign);
}

void uiSignUpdateCredentialThresholdDisplay(volatile unsigned int *flags) {
    // Create tag-value pairs for the content
    uint8_t pairIndex = 0;

    // Add threshold information
    pairs[pairIndex].item = "Threshold";
    pairs[pairIndex].value = (char *)global.signCredentialDeploymentContext.threshold;
    pairIndex++;

    // Create the page content
    nbgl_contentTagValueList_t content;
    content.nbPairs = pairIndex;
    content.pairs = pairs;
    content.smallCaseForValue = false;
    content.nbMaxLinesForValue = 0;
    content.startIndex = 0;

    // Setup the review screen - this is the final step so use nbgl_useCaseReview
    nbgl_useCaseReview(TYPE_TRANSACTION,
                       &content,
                       &ICON_APP_HOME,
                       "Review Transaction",
                       NULL,  // No subtitle
                       "Sign transaction",
                       review_choice_sign);

    *flags |= IO_ASYNCH_REPLY;
}

void uiSignUpdateCredentialInitialDisplay(volatile unsigned int *flags) {
    // Create tag-value pairs for the content
    uint8_t pairIndex = 0;

    // Add sender address
    pairs[pairIndex].item = "Sender";
    pairs[pairIndex].value = (char *)global_account_sender.sender;
    pairIndex++;

    // Create the page content
    nbgl_contentTagValueList_t content;
    content.nbPairs = pairIndex;
    content.pairs = pairs;
    content.smallCaseForValue = false;
    content.nbMaxLinesForValue = 0;
    content.startIndex = 0;

    // Setup the review screen
    nbgl_useCaseReviewLight(TYPE_OPERATION,
                            &content,
                            &ICON_APP_HOME,
                            "Review",
                            "details",
                            "Continue with transaction",
                            sendSuccessNoIdleCallback);

    *flags |= IO_ASYNCH_REPLY;
}

void uiSignUpdateCredentialIdDisplay(volatile unsigned int *flags) {
    // Create tag-value pairs for the content
    uint8_t pairIndex = 0;

    // Add credential ID information
    pairs[pairIndex].item = "Credential ID";
    pairs[pairIndex].value = (char *)global.signCredentialDeploymentContext.credentialId;
    pairIndex++;

    // Create the page content
    nbgl_contentTagValueList_t content;
    content.nbPairs = pairIndex;
    content.pairs = pairs;
    content.smallCaseForValue = false;
    content.nbMaxLinesForValue = 0;
    content.startIndex = 0;

    // Setup the review screen
    nbgl_useCaseReviewLight(TYPE_OPERATION,
                            &content,
                            &ICON_APP_HOME,
                            "Review",
                            "details",
                            "Continue with transaction",
                            sendSuccessNoIdleCallback);

    *flags |= IO_ASYNCH_REPLY;
}

void uiSignCredentialDeploymentVerificationKeyDisplay(volatile unsigned int *flags) {
    // Setup data to display
    uint8_t pairIndex = 0;
    pairs[pairIndex].item = "Public key";
    pairs[pairIndex].value = (char *)global.signCredentialDeploymentContext.accountVerificationKey;
    pairIndex++;

    // Create the page content
    nbgl_contentTagValueList_t content;
    content.nbPairs = pairIndex;
    content.pairs = pairs;
    content.smallCaseForValue = false;
    content.nbMaxLinesForValue = 0;
    content.startIndex = 0;
    // Setup the review screen
    nbgl_useCaseReviewLight(TYPE_OPERATION,
                            &content,
                            &ICON_APP_HOME,
                            "Review",
                            "details",
                            "Continue with transaction",
                            processNextVerificationKeyNoIdleCallback);
    *flags |= IO_ASYNCH_REPLY;
}

void uiSignCredentialDeploymentNewIntroDisplay(void) {
    // Setup data to display
    uint8_t pairIndex = 0;
    pairs[pairIndex].item = "Public key";
    pairs[pairIndex].value = (char *)global.signCredentialDeploymentContext.accountVerificationKey;
    pairIndex++;
    pairs[pairIndex].item = "Signature threshold";
    pairs[pairIndex].value = (char *)global.signCredentialDeploymentContext.signatureThreshold;
    pairIndex++;
    pairs[pairIndex].item = "RegIdCred";
    pairs[pairIndex].value = (char *)global.signCredentialDeploymentContext.regIdCred;
    pairIndex++;
    pairs[pairIndex].item = "Identity provider";
    pairs[pairIndex].value = (char *)global.signCredentialDeploymentContext.identityProviderIndex;
    pairIndex++;
    pairs[pairIndex].item = "AR threshold";
    pairs[pairIndex].value =
        (char *)global.signCredentialDeploymentContext.anonymityRevocationThreshold;
    pairIndex++;
    pairs[pairIndex].item = "AR identity";
    pairs[pairIndex].value = (char *)global.signCredentialDeploymentContext.arIdentity;
    pairIndex++;
    pairs[pairIndex].item = "Encrypted Share";
    pairs[pairIndex].value = (char *)global.signCredentialDeploymentContext.encIdCredPubShare;
    pairIndex++;

    // Create the page content
    nbgl_contentTagValueList_t content;
    content.nbPairs = pairIndex;
    content.pairs = pairs;
    content.smallCaseForValue = false;
    content.nbMaxLinesForValue = 0;
    content.startIndex = 0;
    // Setup the review screen
    nbgl_useCaseReview(TYPE_TRANSACTION,
                       &content,
                       &ICON_APP_HOME,
                       "Review Transaction",
                       NULL,
                       "Sign details",
                       review_choice_sign);
}

void uiSignCredentialDeploymentExistingIntroDisplay(void) {
    // Setup data to display
    uint8_t pairIndex = 0;
    pairs[pairIndex].item = "Public key";
    pairs[pairIndex].value = (char *)global.signCredentialDeploymentContext.accountVerificationKey;
    pairIndex++;
    pairs[pairIndex].item = "Signature threshold";
    pairs[pairIndex].value = (char *)global.signCredentialDeploymentContext.signatureThreshold;
    pairIndex++;
    pairs[pairIndex].item = "RegIdCred";
    pairs[pairIndex].value = (char *)global.signCredentialDeploymentContext.regIdCred;
    pairIndex++;
    pairs[pairIndex].item = "Identity provider";
    pairs[pairIndex].value = (char *)global.signCredentialDeploymentContext.identityProviderIndex;
    pairIndex++;
    pairs[pairIndex].item = "AR threshold";
    pairs[pairIndex].value =
        (char *)global.signCredentialDeploymentContext.anonymityRevocationThreshold;
    pairIndex++;
    pairs[pairIndex].item = "AR identity";
    pairs[pairIndex].value = (char *)global.signCredentialDeploymentContext.arIdentity;
    pairIndex++;
    pairs[pairIndex].item = "Encrypted Share";
    pairs[pairIndex].value = (char *)global.signCredentialDeploymentContext.encIdCredPubShare;
    pairIndex++;
    pairs[pairIndex].item = "Address";
    pairs[pairIndex].value = (char *)global.signCredentialDeploymentContext.accountAddress;
    pairIndex++;

    // Create the page content
    nbgl_contentTagValueList_t content;
    content.nbPairs = pairIndex;
    content.pairs = pairs;
    content.smallCaseForValue = false;
    content.nbMaxLinesForValue = 0;
    content.startIndex = 0;
    // Setup the review screen
    nbgl_useCaseReview(TYPE_TRANSACTION,
                       &content,
                       &ICON_APP_HOME,
                       "Review Transaction",
                       NULL,
                       "Sign details",
                       review_choice_sign);
}

void uiSignCredentialDeploymentNewDisplay(void) {
    // Setup data to display
    uint8_t pairIndex = 0;
    pairs[pairIndex].item = "Public key";
    pairs[pairIndex].value = (char *)global.signCredentialDeploymentContext.accountVerificationKey;
    pairIndex++;
    pairs[pairIndex].item = "Signature threshold";
    pairs[pairIndex].value = (char *)global.signCredentialDeploymentContext.signatureThreshold;
    pairIndex++;
    pairs[pairIndex].item = "RegIdCred";
    pairs[pairIndex].value = (char *)global.signCredentialDeploymentContext.regIdCred;
    pairIndex++;
    pairs[pairIndex].item = "Identity provider";
    pairs[pairIndex].value = (char *)global.signCredentialDeploymentContext.identityProviderIndex;
    pairIndex++;
    pairs[pairIndex].item = "AR threshold";
    pairs[pairIndex].value =
        (char *)global.signCredentialDeploymentContext.anonymityRevocationThreshold;
    pairIndex++;
    pairs[pairIndex].item = "AR identity";
    pairs[pairIndex].value = (char *)global.signCredentialDeploymentContext.arIdentity;
    pairIndex++;
    pairs[pairIndex].item = "Encrypted Share";
    pairs[pairIndex].value = (char *)global.signCredentialDeploymentContext.encIdCredPubShare;
    pairIndex++;

    // Create the page content
    nbgl_contentTagValueList_t content;
    content.nbPairs = pairIndex;
    content.pairs = pairs;
    content.smallCaseForValue = false;
    content.nbMaxLinesForValue = 0;
    content.startIndex = 0;
    // Setup the review screen
    nbgl_useCaseReview(TYPE_TRANSACTION,
                       &content,
                       &ICON_APP_HOME,
                       "Review Transaction",
                       NULL,
                       "Sign details",
                       review_choice_sign);
}

void uiSignCredentialDeploymentExistingDisplay(void) {
    // Setup data to display
    uint8_t pairIndex = 0;
    pairs[pairIndex].item = "Public key";
    pairs[pairIndex].value = (char *)global.signCredentialDeploymentContext.accountVerificationKey;
    pairIndex++;
    pairs[pairIndex].item = "Signature threshold";
    pairs[pairIndex].value = (char *)global.signCredentialDeploymentContext.signatureThreshold;
    pairIndex++;
    pairs[pairIndex].item = "RegIdCred";
    pairs[pairIndex].value = (char *)global.signCredentialDeploymentContext.regIdCred;
    pairIndex++;
    pairs[pairIndex].item = "Identity provider";
    pairs[pairIndex].value = global.signCredentialDeploymentContext.identityProviderIndex;
    pairIndex++;
    pairs[pairIndex].item = "AR threshold";
    pairs[pairIndex].value =
        (char *)global.signCredentialDeploymentContext.anonymityRevocationThreshold;
    pairIndex++;
    pairs[pairIndex].item = "AR identity";
    pairs[pairIndex].value = (char *)global.signCredentialDeploymentContext.arIdentity;
    pairIndex++;
    pairs[pairIndex].item = "Encrypted Share";
    pairs[pairIndex].value = (char *)global.signCredentialDeploymentContext.encIdCredPubShare;
    pairIndex++;
    pairs[pairIndex].item = "Address";
    pairs[pairIndex].value = (char *)global.signCredentialDeploymentContext.accountAddress;
    pairIndex++;

    // Create the page content
    nbgl_contentTagValueList_t content;
    content.nbPairs = pairIndex;
    content.pairs = pairs;
    content.smallCaseForValue = false;
    content.nbMaxLinesForValue = 0;
    content.startIndex = 0;
    // Setup the review screen
    nbgl_useCaseReview(TYPE_TRANSACTION,
                       &content,
                       &ICON_APP_HOME,
                       "Review Transaction",
                       NULL,
                       "Sign details",
                       review_choice_sign);
}

void uiSignCredentialDeploymentVerificationKeyFlowDisplay(volatile unsigned int *flags) {
    // Setup data to display
    uint8_t pairIndex = 0;
    pairs[pairIndex].item = "Public key";
    pairs[pairIndex].value = (char *)global.signCredentialDeploymentContext.accountVerificationKey;
    pairIndex++;

    // Create the page content
    nbgl_contentTagValueList_t content;
    content.nbPairs = pairIndex;
    content.pairs = pairs;
    content.smallCaseForValue = false;
    content.nbMaxLinesForValue = 0;
    content.startIndex = 0;
    // Setup the review screen
    nbgl_useCaseReviewLight(TYPE_OPERATION,
                            &content,
                            &ICON_APP_HOME,
                            "Review",
                            "details",
                            "Continue with transaction",
                            processNextVerificationKeyNoIdleCallback);
    *flags |= IO_ASYNCH_REPLY;
}

void uiSignPublicInformationForIpCompleteDisplay(void) {
    // Setup data to display
    uint8_t pairIndex = 0;
    pairs[pairIndex].item = "Public key";
    pairs[pairIndex].value = (char *)global.signPublicInformationForIp.publicKey;
    pairIndex++;
    pairs[pairIndex].item = "Key type";
    pairs[pairIndex].value = global.signPublicInformationForIp.keyType;
    pairIndex++;
    pairs[pairIndex].item = "Signature threshold";
    pairs[pairIndex].value = (char *)global.signPublicInformationForIp.threshold;
    pairIndex++;
    pairs[pairIndex].item = "Id Cred Pub";
    pairs[pairIndex].value = (char *)global.signPublicInformationForIp.idCredPub;
    pairIndex++;
    pairs[pairIndex].item = "Credential ID";
    pairs[pairIndex].value = (char *)global.signPublicInformationForIp.credId;
    pairIndex++;
    // Create the page content
    nbgl_contentTagValueList_t content;
    content.nbPairs = pairIndex;
    content.pairs = pairs;
    content.smallCaseForValue = false;
    content.nbMaxLinesForValue = 0;
    content.startIndex = 0;
    // Setup the review screen
    nbgl_useCaseReview(TYPE_TRANSACTION,
                       &content,
                       &ICON_APP_HOME,
                       "Review identity",
                       "provider info",
                       "Sign identity",
                       review_choice_sign);
}
void uiReviewPublicInformationForIpDisplay(void) {
    // Setup data to display
    uint8_t pairIndex = 0;
    pairs[pairIndex].item = "Public key";
    pairs[pairIndex].value = global.signPublicInformationForIp.publicKey;
    pairIndex++;
    pairs[pairIndex].item = "Key type";
    pairs[pairIndex].value = global.signPublicInformationForIp.keyType;
    pairIndex++;
    pairs[pairIndex].item = "Id Cred Pub";
    pairs[pairIndex].value = (char *)global.signPublicInformationForIp.idCredPub;
    pairIndex++;
    pairs[pairIndex].item = "Credential ID";
    pairs[pairIndex].value = (char *)global.signPublicInformationForIp.credId;
    pairIndex++;

    // Create the page content
    nbgl_contentTagValueList_t content;
    content.nbPairs = pairIndex;
    content.pairs = pairs;
    content.smallCaseForValue = false;
    content.nbMaxLinesForValue = 0;
    content.startIndex = 0;
    // Setup the review screen
    nbgl_useCaseReviewLight(TYPE_OPERATION,
                            &content,
                            &ICON_APP_HOME,
                            "Review identity",
                            "provider info",
                            "Continue reviewing info",
                            sendSuccessNoIdleCallback);
}

void uiSignPublicInformationForIpFinalDisplay(void) {
    // Setup data to display
    uint8_t pairIndex = 0;
    pairs[pairIndex].item = "Public key";
    pairs[pairIndex].value = (char *)global.signPublicInformationForIp.publicKey;
    pairIndex++;
    pairs[pairIndex].item = "Key type";
    pairs[pairIndex].value = global.signPublicInformationForIp.keyType;
    pairIndex++;
    pairs[pairIndex].item = "Signature threshold";
    pairs[pairIndex].value = (char *)global.signPublicInformationForIp.threshold;
    pairIndex++;

    // Create the page content
    nbgl_contentTagValueList_t content;
    content.nbPairs = pairIndex;
    content.pairs = pairs;
    content.smallCaseForValue = false;
    content.nbMaxLinesForValue = 0;
    content.startIndex = 0;
    // Setup the review screen
    nbgl_useCaseReview(TYPE_TRANSACTION,
                       &content,
                       &ICON_APP_HOME,
                       "Review identity",
                       NULL,  // No subtitle
                       "Sign identity",
                       review_choice_sign);
}

void uiSignPublicInformationForIpPublicKeyDisplay(void) {
    // Setup data to display
    uint8_t pairIndex = 0;
    pairs[pairIndex].item = "Public key";
    pairs[pairIndex].value = global.signPublicInformationForIp.publicKey;
    pairIndex++;
    pairs[pairIndex].item = "Key type";
    pairs[pairIndex].value = global.signPublicInformationForIp.keyType;
    pairIndex++;

    // Create the page content
    nbgl_contentTagValueList_t content;
    content.nbPairs = pairIndex;
    content.pairs = pairs;
    content.smallCaseForValue = false;
    content.nbMaxLinesForValue = 0;
    content.startIndex = 0;
    // Setup the review screen
    nbgl_useCaseReviewLight(TYPE_OPERATION,
                            &content,
                            &ICON_APP_HOME,
                            "Review identity",
                            NULL,  // No subtitle
                            "Continue reviewing info",
                            sendSuccessNoIdleCallback);
}

void uiRegisterDataInitialDisplay(volatile unsigned int *flags) {
    uint8_t pairIndex = 0;
    pairs[pairIndex].item = "Sender";
    pairs[pairIndex].value = (char *)global_account_sender.sender;
    pairIndex++;
    // Create the page content
    nbgl_contentTagValueList_t content;
    content.nbPairs = pairIndex;
    content.pairs = pairs;
    content.smallCaseForValue = false;
    content.nbMaxLinesForValue = 0;
    content.startIndex = 0;
    // Setup the review screen
    nbgl_useCaseReviewLight(TYPE_OPERATION,
                            &content,
                            &ICON_APP_HOME,
                            "Review transaction",
                            NULL,  // No subtitle
                            "Continue with transaction",
                            sendSuccessNoIdleCallback);
    *flags |= IO_ASYNCH_REPLY;
}

void uiRegisterDataPayloadDisplay(volatile unsigned int *flags) {
    // Get context from global state
    signRegisterData_t *ctx = &global.withDataBlob.signRegisterData;

    uint8_t index = 0;
    pairs[index].item = "Data";
    pairs[index].value = (char *)global.withDataBlob.cborContext.display;
    index++;
    // Create the page content
    nbgl_contentTagValueList_t content;
    content.nbPairs = index;
    content.pairs = pairs;
    content.smallCaseForValue = false;
    content.nbMaxLinesForValue = 0;
    content.startIndex = 0;

    if (ctx->dataLength > 0) {
        // Setup the review screen
        nbgl_useCaseReviewLight(TYPE_OPERATION,
                                &content,
                                &ICON_APP_HOME,
                                "Review Data",
                                NULL,  // No subtitle
                                "Continue with transaction",
                                sendSuccessNoIdleCallback);
    } else {
        nbgl_useCaseReview(TYPE_TRANSACTION,
                           &content,
                           &ICON_APP_HOME,
                           "Review Data",
                           NULL,  // No subtitle
                           "Sign transaction",
                           review_choice_sign);
    }

    *flags |= IO_ASYNCH_REPLY;
}

void startTransferDisplay(bool displayMemo, volatile unsigned int *flags) {
    uint8_t index = 0;
    pairs[index].item = "Sender";
    pairs[index].value = (char *)global_account_sender.sender;
    index++;
    pairs[index].item = "Recipient";
    pairs[index].value = (char *)global.withDataBlob.signTransferContext.displayStr;
    index++;
    pairs[index].item = "Amount";
    pairs[index].value = (char *)global.withDataBlob.signTransferContext.displayAmount;
    index++;
    if (displayMemo) {
        pairs[index].item = "Memo";
        pairs[index].value = (char *)global.withDataBlob.cborContext.display;
        index++;
    }

    // Create the page content
    nbgl_contentTagValueList_t content;
    content.nbPairs = index;
    content.pairs = pairs;
    content.smallCaseForValue = false;
    content.nbMaxLinesForValue = 0;
    content.startIndex = 0;
    // Setup the review screen
    nbgl_useCaseReview(TYPE_TRANSACTION,
                       &content,
                       &ICON_APP_HOME,
                       "Review Transfer",
                       NULL,  // No subtitle
                       "Sign Transfer",
                       review_choice_sign);

    *flags |= IO_ASYNCH_REPLY;
}

void uiSignTransferToPublicDisplay(volatile unsigned int *flags) {
    // Setup data to display
    uint8_t pairIndex = 0;
    pairs[pairIndex].item = "Sender";
    pairs[pairIndex].value = (char *)global_account_sender.sender;
    pairIndex++;
    pairs[pairIndex].item = "Unshield amount";
    pairs[pairIndex].value = (char *)global.signTransferToPublic.amount;
    pairIndex++;
    pairs[pairIndex].item = "Recipient";
    pairs[pairIndex].value = (char *)global.signTransferToPublic.recipientAddress;
    pairIndex++;

    // Create the page content
    nbgl_contentTagValueList_t content;
    content.nbPairs = pairIndex;
    content.pairs = pairs;
    content.smallCaseForValue = false;
    content.nbMaxLinesForValue = 0;
    content.startIndex = 0;
    // Setup the review screen
    nbgl_useCaseReview(TYPE_TRANSACTION,
                       &content,
                       &ICON_APP_HOME,
                       "Review Transaction",
                       NULL,  // No subtitle
                       "Sign transaction",
                       review_choice_sign);
    *flags |= IO_ASYNCH_REPLY;
}

// Add this wrapper function before uiSignScheduledTransferPairFlowDisplay
static void processNextScheduledAmountWrapper(bool choice) {
    if (choice) {
        processNextScheduledAmount(ctx_sign_transfer_with_schedule->buffer);
    } else {
        sendUserRejection();
    }
}

void uiSignScheduledTransferPairFlowDisplay(void) {
    // Setup data to display
    uint8_t pairIndex = 0;
    pairs[pairIndex].item = "Release time (UTC)";
    pairs[pairIndex].value =
        (char *)global.withDataBlob.signTransferWithScheduleContext.displayTimestamp;
    pairIndex++;
    pairs[pairIndex].item = "Amount";
    pairs[pairIndex].value = (char *)global.withDataBlob.signTransferWithScheduleContext.displayStr;
    pairIndex++;

    // Create the page content
    nbgl_contentTagValueList_t content;
    content.nbPairs = pairIndex;
    content.pairs = pairs;
    content.smallCaseForValue = false;
    content.nbMaxLinesForValue = 0;
    content.startIndex = 0;
    // Setup the review screen
    nbgl_useCaseReviewLight(TYPE_TRANSACTION,
                            &content,
                            &ICON_APP_HOME,
                            "Review Scheduled release",
                            NULL,  // No subtitle
                            "Show next release",
                            processNextScheduledAmountWrapper);
}

void uiSignScheduledTransferPairFlowSignDisplay(void) {
    // Setup data to display
    uint8_t pairIndex = 0;
    pairs[pairIndex].item = "Release time (UTC)";
    pairs[pairIndex].value =
        (char *)global.withDataBlob.signTransferWithScheduleContext.displayTimestamp;
    pairIndex++;
    pairs[pairIndex].item = "Amount";
    pairs[pairIndex].value = (char *)global.withDataBlob.signTransferWithScheduleContext.displayStr;
    pairIndex++;

    // Create the page content
    nbgl_contentTagValueList_t content;
    content.nbPairs = pairIndex;
    content.pairs = pairs;
    content.smallCaseForValue = false;
    content.nbMaxLinesForValue = 0;
    content.startIndex = 0;
    // Setup the review screen
    nbgl_useCaseReview(TYPE_TRANSACTION,
                       &content,
                       &ICON_APP_HOME,
                       "Review Scheduled release",
                       NULL,  // No subtitle
                       "Sign transaction",
                       review_choice_sign);
}

void uiVerifyAddress(volatile unsigned int *flags) {
    uint8_t index = 0;
    pairs[index].item = "Identity";
    pairs[index].value = (char *)global.verifyAddressContext.display;
    index++;

    // Create the page content
    nbgl_contentTagValueList_t content;
    content.nbPairs = index;
    content.pairs = pairs;
    content.smallCaseForValue = false;
    content.nbMaxLinesForValue = 0;
    content.startIndex = 0;
    // Setup the review screen
    nbgl_useCaseAddressReview((char *)global.verifyAddressContext.address,
                              &content,
                              &ICON_APP_HOME,
                              "Verify Address",
                              NULL,  // No subtitle
                              review_verify_address);
    *flags |= IO_ASYNCH_REPLY;
}

void startInitialScheduledTransferDisplay(bool displayMemo) {
    uint8_t index = 0;
    pairs[index].item = "Sender";
    pairs[index].value = (char *)global_account_sender.sender;
    index++;
    pairs[index].item = "Recipient";
    pairs[index].value = (char *)global.withDataBlob.signTransferWithScheduleContext.displayStr;
    index++;
    if (displayMemo) {
        pairs[index].item = "Memo";
        pairs[index].value = (char *)global.withDataBlob.cborContext.display;
        index++;
    }
    // Create the page content
    nbgl_contentTagValueList_t content;
    content.nbPairs = index;
    content.pairs = pairs;
    content.smallCaseForValue = false;
    content.nbMaxLinesForValue = 0;
    content.startIndex = 0;
    // Setup the review screen
    nbgl_useCaseReviewLight(TYPE_TRANSACTION,
                            &content,
                            &ICON_APP_HOME,
                            "Review Transfer with schedule",
                            NULL,  // No subtitle
                            "Continue with transfer",
                            keep_going_with_transaction);
}

void uiDeployModuleDisplay(void) {
    pairs[0].item = "Sender";
    pairs[0].value = (char *)global_account_sender.sender;
    pairs[1].item = "Version";
    pairs[1].value = (char *)global.deployModule.versionDisplay;

    // Create the page content
    nbgl_contentTagValueList_t content;
    content.nbPairs = 2;
    content.pairs = pairs;
    content.smallCaseForValue = false;
    content.nbMaxLinesForValue = 0;
    content.startIndex = 0;
    // Setup the review screen
    nbgl_useCaseReview(TYPE_TRANSACTION,
                       &content,
                       &ICON_APP_HOME,
                       "Review Transaction \nto deploy module",
                       NULL,  // No subtitle
                       "Sign transaction\nto deploy module",
                       review_choice_sign);
}

void uiUpdateContractDisplay(void) {
    uint8_t pairIndex = 0;
    pairs[pairIndex].item = "Sender";
    pairs[pairIndex].value = (char *)global_account_sender.sender;
    pairIndex++;
    pairs[pairIndex].item = "Amount";
    pairs[pairIndex].value = (char *)global.updateContract.amountDisplay;
    pairIndex++;
    pairs[pairIndex].item = "Index";
    pairs[pairIndex].value = (char *)global.updateContract.indexDisplay;
    pairIndex++;
    pairs[pairIndex].item = "Sub index";
    pairs[pairIndex].value = (char *)global.updateContract.subIndexDisplay;
    pairIndex++;
    // Create the page content
    nbgl_contentTagValueList_t content;
    content.nbPairs = pairIndex;
    content.pairs = pairs;
    content.smallCaseForValue = false;
    content.nbMaxLinesForValue = 0;
    content.startIndex = 0;
    // Setup the review screen
    nbgl_useCaseReview(TYPE_TRANSACTION,
                       &content,
                       &ICON_APP_HOME,
                       "Review Transaction \nto update contract",
                       NULL,  // No subtitle
                       "Sign transaction\nto update contract",
                       review_choice_sign);
}

void uiInitContractDisplay(void) {
    uint8_t pairIndex = 0;
    pairs[pairIndex].item = "Sender";
    pairs[pairIndex].value = (char *)global_account_sender.sender;
    pairIndex++;
    pairs[pairIndex].item = "Amount";
    pairs[pairIndex].value = (char *)global.initContract.amountDisplay;
    pairIndex++;
    pairs[pairIndex].item = "Module ref";
    pairs[pairIndex].value = (char *)global.initContract.moduleRefDisplay;
    pairIndex++;
    // Create the page content
    nbgl_contentTagValueList_t content;
    content.nbPairs = pairIndex;
    content.pairs = pairs;
    content.smallCaseForValue = false;
    content.nbMaxLinesForValue = 0;
    content.startIndex = 0;
    // Setup the review screen
    nbgl_useCaseReview(TYPE_TRANSACTION,
                       &content,
                       &ICON_APP_HOME,
                       "Review Transaction \nto update contract",
                       NULL,  // No subtitle
                       "Sign transaction\nto update contract",
                       review_choice_sign);
}

void uiPltOperationDisplay(void) {
    uint8_t pairIndex = 0;
    pairs[pairIndex].item = "Sender";
    pairs[pairIndex].value = (char *)global_account_sender.sender;
    pairIndex++;
    pairs[pairIndex].item = "Token ID";
    pairs[pairIndex].value = (char *)global.withDataBlob.signPLTContext.tokenId;
    pairIndex++;

    // Use parsed operation data if available, otherwise fall back to raw display
    if (global.withDataBlob.signPLTContext.parsedOperation.isParsed) {
        uint8_t opCount = global.withDataBlob.signPLTContext.parsedOperation.operationCount;

        // For NBGL, support structured display for up to 10 operations
        // For more operations, fall back to JSON display for better user experience
        if (opCount > 10) {
            PRINTF("Too many operations (%d) for individual display, using JSON fallback\n",
                   opCount);
            pairs[pairIndex].item = "PLT Operation(s)";
            pairs[pairIndex].value = (char *)global.withDataBlob.signPLTContext.pltOperationDisplay;
            pairIndex++;
        } else {
            static char opTitles[MAX_PLT_OPERATIONS][32];
            static char amountTitles[MAX_PLT_OPERATIONS][32];
            static char recipientTitles[MAX_PLT_OPERATIONS][32];
            static char targetTitles[MAX_PLT_OPERATIONS][32];

            // Display each operation individually

            for (uint8_t i = 0; i < opCount && i < MAX_PLT_OPERATIONS; i++) {
                // Operation type - always shown
                if (opCount == 1) {
                    snprintf(opTitles[i], sizeof(opTitles[i]), "Operation");
                } else {
                    snprintf(opTitles[i], sizeof(opTitles[i]), "Operation %d", i + 1);
                }
                if (pairIndex < 32) {
                    pairs[pairIndex].item = opTitles[i];
                    pairs[pairIndex].value =
                        (char *)global.withDataBlob.signPLTContext.parsedOperation.operations[i]
                            .operationType;
                    pairIndex++;
                } else {
                    PRINTF("Pairs array full, skipping remaining operations\n");
                    break;
                }
                PRINTF("--------------------------------\n");
                PRINTF("Available fields: %d\n",
                       global.withDataBlob.signPLTContext.parsedOperation.operations[i]
                           .availableFields);
                PRINTF("Match PLT_FIELD_AMOUNT: %d\n",
                       global.withDataBlob.signPLTContext.parsedOperation.operations[i]
                               .availableFields &
                           PLT_FIELD_AMOUNT);
                PRINTF("Match PLT_FIELD_RECIPIENT: %d\n",
                       global.withDataBlob.signPLTContext.parsedOperation.operations[i]
                               .availableFields &
                           PLT_FIELD_RECIPIENT);
                PRINTF("Match PLT_FIELD_TARGET: %d\n",
                       global.withDataBlob.signPLTContext.parsedOperation.operations[i]
                               .availableFields &
                           PLT_FIELD_TARGET);
                PRINTF(
                    "Operation type: %s\n",
                    global.withDataBlob.signPLTContext.parsedOperation.operations[i].operationType);
                PRINTF("Amount: %s\n",
                       global.withDataBlob.signPLTContext.parsedOperation.operations[i].amount);
                PRINTF("Recipient: %s\n",
                       global.withDataBlob.signPLTContext.parsedOperation.operations[i].recipient);
                PRINTF("Target: %s\n",
                       global.withDataBlob.signPLTContext.parsedOperation.operations[i].target);
                PRINTF("--------------------------------\n");
                // Amount field - only if available
                if ((global.withDataBlob.signPLTContext.parsedOperation.operations[i]
                         .availableFields &
                     PLT_FIELD_AMOUNT) &&
                    pairIndex < 32) {
                    if (opCount == 1) {
                        snprintf(amountTitles[i], sizeof(amountTitles[i]), "Amount");
                    } else {
                        snprintf(amountTitles[i], sizeof(amountTitles[i]), "Amount %d", i + 1);
                    }
                    pairs[pairIndex].item = amountTitles[i];
                    pairs[pairIndex].value =
                        (char *)global.withDataBlob.signPLTContext.parsedOperation.operations[i]
                            .amount;
                    pairIndex++;
                }

                // Recipient field - only if available
                if ((global.withDataBlob.signPLTContext.parsedOperation.operations[i]
                         .availableFields &
                     PLT_FIELD_RECIPIENT) &&
                    pairIndex < 32) {
                    if (opCount == 1) {
                        snprintf(recipientTitles[i], sizeof(recipientTitles[i]), "Recipient");
                    } else {
                        snprintf(recipientTitles[i],
                                 sizeof(recipientTitles[i]),
                                 "Recipient %d",
                                 i + 1);
                    }
                    pairs[pairIndex].item = recipientTitles[i];
                    pairs[pairIndex].value =
                        (char *)global.withDataBlob.signPLTContext.parsedOperation.operations[i]
                            .recipient;
                    pairIndex++;
                }

                // Target field - only if available
                if ((global.withDataBlob.signPLTContext.parsedOperation.operations[i]
                         .availableFields &
                     PLT_FIELD_TARGET) &&
                    pairIndex < 32) {
                    if (opCount == 1) {
                        snprintf(targetTitles[i], sizeof(targetTitles[i]), "Target");
                    } else {
                        snprintf(targetTitles[i], sizeof(targetTitles[i]), "Target %d", i + 1);
                    }
                    pairs[pairIndex].item = targetTitles[i];
                    pairs[pairIndex].value =
                        (char *)global.withDataBlob.signPLTContext.parsedOperation.operations[i]
                            .target;
                    pairIndex++;
                }
            }

            PRINTF("Displaying %d individual PLT operations\n", opCount);
        }
    } else {
        // Fallback to raw display
        pairs[pairIndex].item = "PLT Operation(s)";
        pairs[pairIndex].value = (char *)global.withDataBlob.signPLTContext.pltOperationDisplay;
        pairIndex++;
        PRINTF("Using fallback display - pltOperationDisplay: %s\n",
               global.withDataBlob.signPLTContext.pltOperationDisplay);
    }

    // Create the page content
    nbgl_contentTagValueList_t content;
    content.nbPairs = pairIndex;
    content.pairs = pairs;
    content.smallCaseForValue = false;
    content.nbMaxLinesForValue = 0;
    content.startIndex = 0;

    // Setup the review screen
    nbgl_useCaseReview(TYPE_TRANSACTION,
                       &content,
                       &C_app_concordium_64px,
                       "Review PLT Operation(s)",
                       NULL,  // No subtitle
                       "Sign PLT Operation(s)",
                       review_choice_sign);
}

#endif
