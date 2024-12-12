#ifdef HAVE_NBGL

#include "os.h"
#include "ux.h"

#include "display.h"
#include "globals.h"
#include "glyphs.h"
#include "menu.h"
#include "getPublicKey.h"
#include "util.h"
#include "sign.h"

#include "nbgl_use_case.h"
accountSender_t global_account_sender;
static nbgl_contentTagValue_t pairs[10];

static void review_choice(bool confirm) {
    // Answer, display a status page and go back to main
    if (confirm) {
        nbgl_useCaseReviewStatus(STATUS_TYPE_ADDRESS_VERIFIED, ui_menu_main);
    } else {
        nbgl_useCaseReviewStatus(STATUS_TYPE_ADDRESS_REJECTED, ui_menu_main);
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

void uiComparePubkey(void) {
    // TODO: Implement this
    nbgl_useCaseAddressReview(global.exportPublicKeyContext.publicKey,
                              NULL,
                              &C_app_concordium_64px,
                              "Compare",
                              NULL,
                              review_choice);
}

void uiGeneratePubkey(volatile unsigned int *flags) {
    nbgl_useCaseAddressReview((char *) global.exportPublicKeyContext.display,  // Address to display
                              NULL,                    // No additional tag-value list
                              &C_app_concordium_64px,  // Icon to display
                              "Public Key",            // Review title
                              NULL,                    // No review subtitle
                              sendPublicKey            // Callback function
    );
    *flags |= IO_ASYNCH_REPLY;
    // TODO: Implement this
}

void uiExportPrivateKey(volatile unsigned int *flags) {
    *flags |= IO_ASYNCH_REPLY;
    // TODO: Implement this
}

void startConfigureBakerCommissionDisplay(void) {
    // Get context from global state
    signConfigureBaker_t *ctx = &global.signConfigureBaker;
    // Create tag-value pairs for the content
    uint8_t pairIndex = 0;

    if (ctx->firstDisplay) {
        // Add sender address
        pairs[pairIndex].item = "Sender";
        pairs[pairIndex].value = (char *) global_account_sender.sender;
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
        pairs[pairIndex].value = (char *) global.signConfigureBaker.commissionRates.transactionFeeCommissionRate;
        pairIndex++;
    }

    if (ctx->hasBakingRewardCommission) {
        pairs[pairIndex].item = "Baking reward";
        pairs[pairIndex].value = (char *) global.signConfigureBaker.commissionRates.bakingRewardCommissionRate;
        pairIndex++;
    }

    if (ctx->hasFinalizationRewardCommission) {
        pairs[pairIndex].item = "Finalization reward";
        pairs[pairIndex].value = (char *) global.signConfigureBaker.commissionRates.finalizationRewardCommissionRate;
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
                        &C_app_concordium_64px,
                        "Review Transaction",
                        NULL,  // No subtitle
                        "Sign Transaction",
                        review_choice_sign);
}

void startConfigureBakerDisplay(void) {
    // Get context from global state
    signConfigureBaker_t *ctx = &global.signConfigureBaker;

    // Create tag-value pairs for the content
    uint8_t pairIndex = 0;
    // Add sender address
    pairs[pairIndex].item = "Sender";
    pairs[pairIndex].value = (char *) global_account_sender.sender;
    pairIndex++;

    ctx->firstDisplay = false;

    if (ctx->hasCapital) {
        if (ctx->capitalRestakeDelegation.stopBaking) {
            pairs[pairIndex].item = "Stop";
            pairs[pairIndex].value = "Baking";
        } else {
            pairs[pairIndex].item = "Amount to stake";
            pairs[pairIndex].value = (char *) global.signConfigureBaker.capitalRestakeDelegation.displayCapital;
        }
        pairIndex++;
    }

    if (ctx->hasRestakeEarnings) {
        pairs[pairIndex].item = "Restake earnings";
        pairs[pairIndex].value = (char *) global.signConfigureBaker.capitalRestakeDelegation.displayRestake;
        pairIndex++;
    }

    if (ctx->hasOpenForDelegation) {
        pairs[pairIndex].item = "Pool status";
        pairs[pairIndex].value = (char *) global.signConfigureBaker.capitalRestakeDelegation.displayOpenForDelegation;
        pairIndex++;
    }

    if (ctx->hasKeys) {
        pairs[pairIndex].item = "Update baker";
        pairs[pairIndex].value = "keys";
        pairIndex++;
    }

    PRINTF("GUI: pairIndex: %d\n", pairIndex);
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
                                &C_app_concordium_64px,
                                "Review Transaction",
                                NULL,  // No subtitle
                                "Continue with transaction",
                                sendSuccessNoIdle);
    } else {
        // Create the page content
        PRINTF("GUI: Sign transaction:");
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
        pairs[pairIndex].value = (char *) global_account_sender.sender;
        pairIndex++;
        ctx->firstDisplay = false;
    }

    if (!lastUrlPage) {
        pairs[pairIndex].item = "URL";
        pairs[pairIndex].value = (char *) global.signConfigureBaker.url.urlDisplay;
        pairIndex++;
    } else {
        if (ctx->url.urlLength == 0) {
            pairs[pairIndex].item = "Empty URL";
            pairs[pairIndex].value = "";
        } else {
            pairs[pairIndex].item = "URL";
            pairs[pairIndex].value = (char *) global.signConfigureBaker.url.urlDisplay;
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
                                &C_app_concordium_64px,
                                "Review Transaction",
                                NULL,  // No subtitle
                                "Continue with transaction",
                                sendSuccessNoIdle);
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
                           &C_app_concordium_64px,
                           "Review Transaction",
                           NULL,  // No subtitle
                           "Sign transaction",
                           review_choice_sign);
    }
}

// TODO: To fix
void startConfigureDelegationDisplay(void) {
    // Get context from global state
    signConfigureDelegationContext_t *ctx = &global.signConfigureDelegation;

    // Create tag-value pairs for the content
    uint8_t pairIndex = 0;
    // Add sender address
    pairs[pairIndex].item = "Sender";
    pairs[pairIndex].value = (char *) global_account_sender.sender;
    pairIndex++;

    // Add capital amount if present
    if (ctx->hasCapital) {
        if (ctx->stopDelegation) {
            pairs[pairIndex].item = "Action";
            pairs[pairIndex].value = "Stop delegation";
        } else {
            pairs[pairIndex].item = "Amount to delegate";
            pairs[pairIndex].value = (char *) ctx->displayCapital;
        }
        pairIndex++;
    }

    // Add restake earnings if present
    if (ctx->hasRestakeEarnings) {
        pairs[pairIndex].item = "Restake earnings";
        pairs[pairIndex].value = (char *) ctx->displayRestake;
        pairIndex++;
    }

    // Add delegation target if present
    if (ctx->hasDelegationTarget) {
        pairs[pairIndex].item = "Delegation target";
        pairs[pairIndex].value = (char *) ctx->displayDelegationTarget;
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
                       &C_app_concordium_64px,
                       "Review Transaction",
                       NULL,  // No subtitle
                       "Sign transaction",
                       review_choice_sign);
}

void uiSignUpdateCredentialThresholdDisplay(volatile unsigned int *flags) {
    *flags |= IO_ASYNCH_REPLY;
    // TODO: Implement this
}

void uiSignUpdateCredentialInitialDisplay(volatile unsigned int *flags) {
    *flags |= IO_ASYNCH_REPLY;
    // TODO: Implement this
}

void uiSignUpdateCredentialIdDisplay(volatile unsigned int *flags) {
    *flags |= IO_ASYNCH_REPLY;
    // TODO: Implement this
}

void uiSignCredentialDeploymentVerificationKeyDisplay(volatile unsigned int *flags) {
    *flags |= IO_ASYNCH_REPLY;
    // TODO: Implement this
}

void uiSignCredentialDeploymentNewIntroDisplay(void) {
    // TODO: Implement this
}

void uiSignCredentialDeploymentExistingIntroDisplay(void) {
    // TODO: Implement this
}

void uiSignCredentialDeploymentNewDisplay(void) {
    // TODO: Implement this
}

void uiSignCredentialDeploymentExistingDisplay(void) {
    // TODO: Implement this
}

void uiSignCredentialDeploymentVerificationKeyFlowDisplay(volatile unsigned int *flags) {
    *flags |= IO_ASYNCH_REPLY;
    // TODO: Implement this
}

void startEncryptedTransferDisplay(bool displayMemo) {
    displayMemo = false;
    // TODO: Implement this
}

void uiSignPublicInformationForIpCompleteDisplay(void) {
    // TODO: Implement this
}

void uiReviewPublicInformationForIpDisplay(void) {
    // TODO: Implement this
}

void uiSignPublicInformationForIpFinalDisplay(void) {
    // TODO: Implement this
}

void uiSignPublicInformationForIpPublicKeyDisplay(void) {
    // TODO: Implement this
}

void uiRegisterDataInitialDisplay(volatile unsigned int *flags) {
    *flags |= IO_ASYNCH_REPLY;
    // TODO: Implement this
}

void uiRegisterDataPayloadDisplay(volatile unsigned int *flags) {
    *flags |= IO_ASYNCH_REPLY;
    // TODO: Implement this
}

void startTransferDisplay(bool displayMemo, volatile unsigned int *flags) {
    *flags |= IO_ASYNCH_REPLY;
    displayMemo = false;
    // TODO: Implement this
}

void uiSignTransferToEncryptedDisplay(volatile unsigned int *flags) {
    *flags |= IO_ASYNCH_REPLY;
    // TODO: Implement this
}

void uiSignTransferToPublicDisplay(volatile unsigned int *flags) {
    // Setup data to display
    uint8_t pairIndex = 0;
    pairs[pairIndex].item = "Sender";
    pairs[pairIndex].value = (char *) global_account_sender.sender;
    pairIndex++;
    pairs[pairIndex].item = "Unshield amount";
    pairs[pairIndex].value = (char *) global.signTransferToPublic.amount;
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
                       &C_app_concordium_64px,
                       "Review Transaction",
                       NULL,  // No subtitle
                       "Sign transaction",
                       review_choice_sign);
    *flags |= IO_ASYNCH_REPLY;
}

void uiSignScheduledTransferPairFlowDisplay(void) {
    // TODO: Implement this
}

void uiSignScheduledTransferPairFlowSignDisplay(void) {
    // TODO: Implement this
}

void uiSignScheduledTransferPairFlowFinalDisplay(void) {
    // TODO: Implement this
}

void uiVerifyAddress(volatile unsigned int *flags) {
    *flags |= IO_ASYNCH_REPLY;
    // TODO: Implement this
}

void startInitialScheduledTransferDisplay(bool displayMemo) {
    displayMemo = false;
    // TODO: Implement this
}

void uiDeployModuleDisplay(void) {
    pairs[0].item = "Sender";
    pairs[0].value = (char *) global_account_sender.sender;
    pairs[1].item = "Version";
    pairs[1].value = (char *) global.deployModule.versionDisplay;

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
                       &C_app_concordium_64px,
                       "Review Transaction \nto deploy module",
                       NULL,  // No subtitle
                       "Sign transaction\nto deploy module",
                       review_choice_sign);
}

void uiUpdateContractDisplay(void) {
    // TODO: Implement this
}

void uiInitContractDisplay(void) {
    uint8_t pairIndex = 0;
    pairs[pairIndex].item = "Sender";
    pairs[pairIndex].value = (char *) global_account_sender.sender;
    pairIndex++;
    pairs[pairIndex].item = "Amount";
    pairs[pairIndex].value = (char *) global.initContract.amountDisplay;
    pairIndex++;
    pairs[pairIndex].item = "Module ref";
    pairs[pairIndex].value = (char *) global.initContract.moduleRefDisplay;
    pairIndex++;
    // Create the page content
    nbgl_contentTagValueList_t content;
    content.nbPairs = 3;
    content.pairs = pairs;
    content.smallCaseForValue = false;
    content.nbMaxLinesForValue = 0;
    content.startIndex = 0;
    // Setup the review screen
    nbgl_useCaseReview(TYPE_TRANSACTION,
                       &content,
                       &C_app_concordium_64px,
                       "Review Transaction \nto update contract",
                       NULL,  // No subtitle
                       "Sign transaction\nto update contract",
                       review_choice_sign);
}

#endif
