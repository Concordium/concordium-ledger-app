#ifdef HAVE_BAGL
#include "globals.h"

accountSender_t global_account_sender;

UX_STEP_NOCB(ux_display_memo_step_nocb,
             bnnn_paging,
             {"Memo", (char *)global.withDataBlob.cborContext.display});

UX_STEP_CB(ux_display_memo_step,
           bnnn_paging,
           sendSuccessNoIdle(),
           {"Memo", (char *)global.withDataBlob.cborContext.display});

UX_FLOW(ux_display_memo, &ux_display_memo_step);

UX_STEP_NOCB(ux_sign_flow_account_sender_view,
             bnnn_paging,
             {.title = "Sender", .text = (char *)global_account_sender.sender});

// UI definitions for comparison of public-key on the device
// with the public-key that the caller received.
UX_STEP_NOCB(ux_sign_compare_public_key_0_step,
             bnnn_paging,
             {.title = "Compare", .text = (char *)global.exportPublicKeyContext.publicKey});
UX_STEP_CB(ux_compare_accept_step, pb, ui_menu_main(), {&C_icon_validate_14, "Accept"});
UX_STEP_CB(ux_compare_decline_step, pb, ui_menu_main(), {&C_icon_crossmark, "Decline"});
UX_FLOW(ux_sign_compare_public_key,
        &ux_sign_compare_public_key_0_step,
        &ux_compare_accept_step,
        &ux_compare_decline_step);

void uiComparePubkey(void) {
    ux_flow_init(0, ux_sign_compare_public_key, NULL);
}

UX_STEP_VALID(ux_decline_step, pb, sendUserRejection(), {&C_icon_crossmark, "Decline"});

// UI definitions for the approval of the generation of a public-key. This prompts the user to
// accept that a public-key will be generated and returned to the computer.
UX_STEP_VALID(ux_generate_public_flow_0_step,
              pnn,
              sendPublicKey(true),
              {&C_icon_validate_14, "Public key", (char *)global.exportPublicKeyContext.display});
UX_FLOW(ux_generate_public_flow, &ux_generate_public_flow_0_step, &ux_decline_step, FLOW_LOOP);

void uiGeneratePubkey(volatile unsigned int *flags) {
    // Display the UI for the public-key flow, where the user can validate that the
    // public-key being generated is the expected one.
    ux_flow_init(0, ux_generate_public_flow, NULL);

    // Tell the main process to wait for a button press.
    *flags |= IO_ASYNCH_REPLY;
}

UX_STEP_NOCB(ux_verify_address_0_step,
             bnnn_paging,
             {.title = "Verify Address", .text = (char *)global.verifyAddressContext.display});

UX_STEP_NOCB(ux_verify_address_1_step,
             bnnn_paging,
             {.title = "Address", .text = (char *)global.verifyAddressContext.address});
UX_STEP_CB(ux_verify_address_approve_step, pb, sendSuccess(0), {&C_icon_validate_14, "Approve"});
UX_STEP_CB(ux_verify_address_reject_step, pb, sendUserRejection(), {&C_icon_crossmark, "Reject"});
UX_FLOW(ux_verify_address,
        &ux_verify_address_0_step,
        &ux_verify_address_1_step,
        &ux_verify_address_approve_step,
        &ux_verify_address_reject_step);

void uiVerifyAddress(volatile unsigned int *flags) {
    ux_flow_init(0, ux_verify_address, NULL);
    *flags |= IO_ASYNCH_REPLY;
}

// Common initial view for signing flows.
UX_STEP_NOCB(ux_sign_flow_shared_review, nn, {"Review", "transaction"});

// Common signature flow for all transactions allowing the user to either sign the transaction hash
// that is currently being processed, or declining to do so (sending back a user rejection error to
// the caller).
UX_STEP_CB(ux_sign_flow_shared_sign,
           pnn,
           buildAndSignTransactionHash(),
           {&C_icon_validate_14, "Sign", "transaction"});
UX_STEP_CB(ux_sign_flow_shared_decline,
           pnn,
           sendUserRejection(),
           {&C_icon_crossmark, "Decline to", "sign transaction"});
UX_FLOW(ux_sign_flow_shared, &ux_sign_flow_shared_sign, &ux_sign_flow_shared_decline);

UX_STEP_NOCB(ux_export_private_key_0_step,
             nn,
             {(char *)global.exportPrivateKeyContext.displayHeader,
              (char *)global.exportPrivateKeyContext.display});
UX_STEP_CB(ux_export_private_key_accept_step,
           pb,
           exportPrivateKey(),
           {&C_icon_validate_14, "Accept"});
UX_STEP_CB(ux_export_private_key_decline_step,
           pb,
           sendUserRejection(),
           {&C_icon_crossmark, "Decline"});
UX_FLOW(ux_export_private_key,
        &ux_export_private_key_0_step,
        &ux_export_private_key_accept_step,
        &ux_export_private_key_decline_step);

void uiExportPrivateKey(volatile unsigned int *flags) {
    ux_flow_init(0, ux_export_private_key, NULL);
    *flags |= IO_ASYNCH_REPLY;
}

// Baker

static signConfigureBaker_t *ctx_conf_baker = &global.signConfigureBaker;

const ux_flow_step_t *ux_sign_configure_baker_first[10];
const ux_flow_step_t *ux_sign_configure_baker_url[6];
const ux_flow_step_t *ux_sign_configure_baker_commission[9];
const ux_flow_step_t *ux_sign_configure_baker_suspended[6];

UX_STEP_NOCB(ux_sign_configure_baker_stop_baking_step, nn, {"Stop", "baking"});

UX_STEP_NOCB(ux_sign_configure_baker_capital_step,
             bnnn_paging,
             {.title = "Amount to stake",
              .text = (char *)global.signConfigureBaker.capitalRestakeDelegation.displayCapital});

UX_STEP_NOCB(ux_sign_configure_baker_restake_step,
             bn,
             {"Restake earnings",
              (char *)global.signConfigureBaker.capitalRestakeDelegation.displayRestake});

UX_STEP_NOCB(ux_sign_configure_baker_open_status_step,
             bn,
             {"Pool status",
              (char *)global.signConfigureBaker.capitalRestakeDelegation.displayOpenForDelegation});

UX_STEP_NOCB(ux_sign_configure_baker_keys_step, nn, {"Update baker", "keys"});

UX_STEP_CB(ux_sign_configure_baker_url_cb_step,
           bnnn_paging,
           sendSuccessNoIdle(),
           {.title = "URL", .text = (char *)global.signConfigureBaker.url.urlDisplay});

UX_STEP_NOCB(ux_sign_configure_baker_url_step,
             bnnn_paging,
             {.title = "URL", .text = (char *)global.signConfigureBaker.url.urlDisplay});

UX_STEP_CB(ux_sign_configure_baker_continue,
           nn,
           sendSuccessNoIdle(),
           {"Continue", "with transaction"});

UX_STEP_NOCB(ux_sign_configure_baker_empty_url_step, bn, {"Empty URL", ""});

UX_STEP_NOCB(ux_sign_configure_baker_commission_rates_step, nn, {"Commission", "rates"});

UX_STEP_NOCB(ux_sign_configure_baker_commission_transaction_fee_step,
             bn,
             {"Transaction fee",
              (char *)global.signConfigureBaker.commissionRates.transactionFeeCommissionRate});

UX_STEP_NOCB(ux_sign_configure_baker_commission_baking_reward_step,
             bn,
             {"Baking reward",
              (char *)global.signConfigureBaker.commissionRates.bakingRewardCommissionRate});

UX_STEP_NOCB(ux_sign_configure_baker_commission_finalization_reward_step,
             bn,
             {"Finalization reward",
              (char *)global.signConfigureBaker.commissionRates.finalizationRewardCommissionRate});

UX_STEP_NOCB(ux_sign_configure_baker_suspended_step,
             bn,
             {"Validator status", (char *)global.signConfigureBaker.suspended});

/**
 * Dynamically builds and initializes the capital, restake earnings, pool status and
 * baker keys display.
 * - Ensures that the UI starts with the shared review transaction screens.
 * - Only displays the parts of the transaction that are set in the transaction, and skips
 *   any optional fields that are not included.
 * - If either the URL or commission rates are in the transaction, then it shows a continue screen
 *   at the end.
 */
void startConfigureBakerDisplay(void) {
    uint8_t index = 0;

    ux_sign_configure_baker_first[index++] = &ux_sign_flow_shared_review;
    ux_sign_configure_baker_first[index++] = &ux_sign_flow_account_sender_view;
    ctx_conf_baker->firstDisplay = false;

    if (ctx_conf_baker->hasCapital) {
        if (ctx_conf_baker->capitalRestakeDelegation.stopBaking) {
            ux_sign_configure_baker_first[index++] = &ux_sign_configure_baker_stop_baking_step;
        } else {
            ux_sign_configure_baker_first[index++] = &ux_sign_configure_baker_capital_step;
        }
    }

    if (ctx_conf_baker->hasRestakeEarnings) {
        ux_sign_configure_baker_first[index++] = &ux_sign_configure_baker_restake_step;
    }

    if (ctx_conf_baker->hasOpenForDelegation) {
        ux_sign_configure_baker_first[index++] = &ux_sign_configure_baker_open_status_step;
    }

    if (ctx_conf_baker->hasKeys) {
        ux_sign_configure_baker_first[index++] = &ux_sign_configure_baker_keys_step;
    }

    // If there are additional steps, then show continue screen. If this is the last step,
    // then show signing screens.
    if (ctx_conf_baker->hasMetadataUrl || hasCommissionRate()) {
        ux_sign_configure_baker_first[index++] = &ux_sign_configure_baker_continue;
    } else {
        ux_sign_configure_baker_first[index++] = &ux_sign_flow_shared_sign;
        ux_sign_configure_baker_first[index++] = &ux_sign_flow_shared_decline;
    }

    ux_sign_configure_baker_first[index++] = FLOW_END_STEP;

    ux_flow_init(0, ux_sign_configure_baker_first, NULL);
}

/**
 * Dynamically builds and initializes the URL display.
 * - If the transaction does not contain any capital, restake earnings, open for delegation or any
 *   baker keys, then it ensures that the UI starts with the shared review transaction screens. As
 *   the same method is used for the pagination of the URL, this is only the case the first time it
 *   is called.
 * - If it is the final part of the URL display and there are no commission rates as part of the
 *   transaction, then it displays the signing / decline screens.
 * - If there are commission rates in the transaction, then it shows a continue screen.
 * - If it is the final part of the URL display, then the URL screen does not have a callback to
 * continue as additional UI elements are added to guide the user forward.
 */
void startConfigureBakerUrlDisplay(bool lastUrlPage) {
    uint8_t index = 0;

    if (ctx_conf_baker->firstDisplay) {
        ux_sign_configure_baker_url[index++] = &ux_sign_flow_shared_review;
        ux_sign_configure_baker_url[index++] = &ux_sign_flow_account_sender_view;
        ctx_conf_baker->firstDisplay = false;
    }

    if (!lastUrlPage) {
        ux_sign_configure_baker_url[index++] = &ux_sign_configure_baker_url_cb_step;
    } else {
        if (ctx_conf_baker->url.urlLength == 0) {
            ux_sign_configure_baker_url[index++] = &ux_sign_configure_baker_empty_url_step;
        } else {
            ux_sign_configure_baker_url[index++] = &ux_sign_configure_baker_url_step;
        }

        // If there are additional steps show the continue screen, otherwise go
        // to signing screens.
        if (hasCommissionRate()) {
            ux_sign_configure_baker_url[index++] = &ux_sign_configure_baker_continue;
        } else {
            ux_sign_configure_baker_url[index++] = &ux_sign_flow_shared_sign;
            ux_sign_configure_baker_url[index++] = &ux_sign_flow_shared_decline;
        }
    }

    ux_sign_configure_baker_url[index++] = FLOW_END_STEP;
    ux_flow_init(0, ux_sign_configure_baker_url, NULL);
}

/**
 * Dynamically builds and initializes the commission display.
 * - If the transaction only contains commission rates, then it ensures that
 *   the UI starts with the shared review transaction screens.
 * - Only shows the commission rates that have been indicated to be part of the transaction.
 * - Shows the signing / decline screens.
 */
void startConfigureBakerCommissionDisplay() {
    uint8_t index = 0;

    if (ctx_conf_baker->firstDisplay) {
        ux_sign_configure_baker_commission[index++] = &ux_sign_flow_shared_review;
        ux_sign_configure_baker_commission[index++] = &ux_sign_flow_account_sender_view;
        ctx_conf_baker->firstDisplay = false;
    }

    if (ctx_conf_baker->hasTransactionFeeCommission || ctx_conf_baker->hasBakingRewardCommission ||
        ctx_conf_baker->hasFinalizationRewardCommission) {
        ux_sign_configure_baker_commission[index++] =
            &ux_sign_configure_baker_commission_rates_step;
    }

    if (ctx_conf_baker->hasTransactionFeeCommission) {
        ux_sign_configure_baker_commission[index++] =
            &ux_sign_configure_baker_commission_transaction_fee_step;
    }

    if (ctx_conf_baker->hasBakingRewardCommission) {
        ux_sign_configure_baker_commission[index++] =
            &ux_sign_configure_baker_commission_baking_reward_step;
    }

    if (ctx_conf_baker->hasFinalizationRewardCommission) {
        ux_sign_configure_baker_commission[index++] =
            &ux_sign_configure_baker_commission_finalization_reward_step;
    }

    if (ctx_conf_baker->hasSuspended) {
        ux_sign_configure_baker_commission[index++] = &ux_sign_configure_baker_continue;
    } else {
        ux_sign_configure_baker_commission[index++] = &ux_sign_flow_shared_sign;
        ux_sign_configure_baker_commission[index++] = &ux_sign_flow_shared_decline;
    }

    ux_sign_configure_baker_commission[index++] = FLOW_END_STEP;
    ux_flow_init(0, ux_sign_configure_baker_commission, NULL);
}

/**
 * Dynamically builds and initializes the suspended display.
 * - If the transaction only contains suspended boolean, then it ensures that
 *   the UI starts with the shared review transaction screens.
 * - Only shows the suspended message that have been indicated to be part of the transaction.
 * - Shows the signing / decline screens.
 */
void startConfigureBakerSuspendedDisplay() {
    uint8_t index = 0;

    if (ctx_conf_baker->firstDisplay) {
        ux_sign_configure_baker_suspended[index++] = &ux_sign_flow_shared_review;
        ux_sign_configure_baker_suspended[index++] = &ux_sign_flow_account_sender_view;
        ctx_conf_baker->firstDisplay = false;
    }

    ux_sign_configure_baker_suspended[index++] = &ux_sign_configure_baker_suspended_step;

    ux_sign_configure_baker_suspended[index++] = &ux_sign_flow_shared_sign;
    ux_sign_configure_baker_suspended[index++] = &ux_sign_flow_shared_decline;

    ux_sign_configure_baker_suspended[index++] = FLOW_END_STEP;
    ux_flow_init(0, ux_sign_configure_baker_suspended, NULL);
}

// Delegation

static signConfigureDelegationContext_t *ctx_conf_delegation = &global.signConfigureDelegation;

// There will at most be 8 UI steps when all 3 optional fields are available.
const ux_flow_step_t *ux_sign_configure_delegation[8];

UX_STEP_NOCB(ux_sign_configure_delegation_capital_step,
             bnnn_paging,
             {.title = "Amount to delegate",
              .text = (char *)global.signConfigureDelegation.displayCapital});

UX_STEP_NOCB(ux_sign_configure_delegation_restake_step,
             bnnn_paging,
             {.title = "Restake earnings",
              .text = (char *)global.signConfigureDelegation.displayRestake});

UX_STEP_NOCB(ux_sign_configure_delegation_pool_step,
             bnnn_paging,
             {.title = "Delegation target",
              .text = (char *)global.signConfigureDelegation.displayDelegationTarget});

UX_STEP_NOCB(ux_sign_configure_delegation_stop_delegation_step, nn, {"Stop", "delegation"});

void startConfigureDelegationDisplay() {
    uint8_t index = 0;

    ux_sign_configure_delegation[index++] = &ux_sign_flow_shared_review;
    ux_sign_configure_delegation[index++] = &ux_sign_flow_account_sender_view;

    if (ctx_conf_delegation->hasCapital) {
        if (ctx_conf_delegation->stopDelegation) {
            ux_sign_configure_delegation[index++] =
                &ux_sign_configure_delegation_stop_delegation_step;
        } else {
            ux_sign_configure_delegation[index++] = &ux_sign_configure_delegation_capital_step;
        }
    }

    if (ctx_conf_delegation->hasRestakeEarnings) {
        ux_sign_configure_delegation[index++] = &ux_sign_configure_delegation_restake_step;
    }

    if (ctx_conf_delegation->hasDelegationTarget) {
        ux_sign_configure_delegation[index++] = &ux_sign_configure_delegation_pool_step;
    }

    ux_sign_configure_delegation[index++] = &ux_sign_flow_shared_sign;
    ux_sign_configure_delegation[index++] = &ux_sign_flow_shared_decline;

    ux_sign_configure_delegation[index++] = FLOW_END_STEP;

    ux_flow_init(0, ux_sign_configure_delegation, NULL);
}

// Credential deployment

UX_STEP_CB(ux_credential_deployment_review_details, nn, sendSuccessNoIdle(), {"Review", "details"});

UX_STEP_CB(ux_update_credentials_initial_flow_1_step,
           nn,
           sendSuccessNoIdle(),
           {"Continue", "with transaction"});

UX_FLOW(ux_update_credentials_initial_flow,
        &ux_sign_flow_shared_review,
        &ux_sign_flow_account_sender_view,
        &ux_update_credentials_initial_flow_1_step);

UX_STEP_NOCB(ux_credential_deployment_verification_key_flow_0_step,
             bnnn_paging,
             {.title = "Public key",
              .text = (char *)global.signCredentialDeploymentContext.accountVerificationKey});

UX_STEP_CB(ux_credential_deployment_verification_key_flow_1_step,
           nn,
           processNextVerificationKey(),
           {"Continue", "with transaction"});

UX_FLOW(ux_credential_deployment_verification_key_flow,
        &ux_credential_deployment_verification_key_flow_0_step,
        &ux_credential_deployment_verification_key_flow_1_step);
UX_FLOW(ux_credential_deployment_verification_key_flow_with_intro,
        &ux_credential_deployment_review_details,
        &ux_credential_deployment_verification_key_flow_0_step,
        &ux_credential_deployment_verification_key_flow_1_step);

UX_STEP_NOCB(ux_credential_deployment_threshold_flow_0_step,
             bn,
             {"Signature threshold",
              (char *)global.signCredentialDeploymentContext.signatureThreshold});
UX_STEP_CB(ux_credential_deployment_threshold_flow_1_step,
           bn,
           sendSuccessNoIdle(),
           {"AR threshold",
            (char *)global.signCredentialDeploymentContext.anonymityRevocationThreshold});

UX_STEP_NOCB(ux_sign_credential_deployment_address_step,
             bnnn_paging,
             {.title = "Address",
              .text = (char *)global.signCredentialDeploymentContext.accountAddress});

UX_STEP_NOCB(ux_sign_credential_deployment_1_step,
             bnnn_paging,
             {.title = "RegIdCred",
              .text = (char *)global.signCredentialDeploymentContext.regIdCred});

UX_STEP_NOCB(ux_sign_credential_deployment_2_step,
             bnnn_paging,
             {.title = "Identity provider",
              .text = (char *)global.signCredentialDeploymentContext.identityProviderIndex});

UX_STEP_NOCB(ux_sign_credential_deployment_3_step,
             bnnn_paging,
             {.title = "AR identity",
              .text = (char *)global.signCredentialDeploymentContext.arIdentity});

UX_STEP_NOCB(ux_sign_credential_deployment_4_step,
             bnnn_paging,
             {.title = "EncryptedShare",
              .text = (char *)global.signCredentialDeploymentContext.encIdCredPubShare});

UX_STEP_CB(ux_sign_credential_deployment_approve_step,
           pnn,
           buildAndSignTransactionHash(),
           {&C_icon_validate_14, "Sign", "details"});
UX_STEP_CB(ux_sign_credential_deployment_reject_step,
           pnn,
           sendUserRejection(),
           {&C_icon_crossmark, "Decline to", "sign details"});

UX_FLOW(ux_sign_credential_deployment_existing_with_intro,
        &ux_credential_deployment_review_details,
        &ux_credential_deployment_verification_key_flow_0_step,
        &ux_credential_deployment_threshold_flow_0_step,
        &ux_credential_deployment_threshold_flow_1_step,
        &ux_sign_credential_deployment_address_step,
        &ux_sign_credential_deployment_1_step,
        &ux_sign_credential_deployment_2_step,
        &ux_sign_credential_deployment_3_step,
        &ux_sign_credential_deployment_4_step,
        &ux_sign_credential_deployment_approve_step,
        &ux_sign_credential_deployment_reject_step);

UX_FLOW(ux_sign_credential_deployment_existing,
        &ux_credential_deployment_verification_key_flow_0_step,
        &ux_credential_deployment_threshold_flow_0_step,
        &ux_credential_deployment_threshold_flow_1_step,
        &ux_sign_credential_deployment_address_step,
        &ux_sign_credential_deployment_1_step,
        &ux_sign_credential_deployment_2_step,
        &ux_sign_credential_deployment_3_step,
        &ux_sign_credential_deployment_4_step,
        &ux_sign_credential_deployment_approve_step,
        &ux_sign_credential_deployment_reject_step);

UX_FLOW(ux_sign_credential_deployment_new_with_intro,
        &ux_credential_deployment_review_details,
        &ux_credential_deployment_verification_key_flow_0_step,
        &ux_credential_deployment_threshold_flow_0_step,
        &ux_credential_deployment_threshold_flow_1_step,
        &ux_sign_credential_deployment_1_step,
        &ux_sign_credential_deployment_2_step,
        &ux_sign_credential_deployment_3_step,
        &ux_sign_credential_deployment_4_step,
        &ux_sign_credential_deployment_approve_step,
        &ux_sign_credential_deployment_reject_step);

UX_FLOW(ux_sign_credential_deployment_new,
        &ux_credential_deployment_verification_key_flow_0_step,
        &ux_credential_deployment_threshold_flow_0_step,
        &ux_credential_deployment_threshold_flow_1_step,
        &ux_sign_credential_deployment_1_step,
        &ux_sign_credential_deployment_2_step,
        &ux_sign_credential_deployment_3_step,
        &ux_sign_credential_deployment_4_step,
        &ux_sign_credential_deployment_approve_step,
        &ux_sign_credential_deployment_reject_step);

UX_STEP_CB(ux_sign_credential_update_id_0_step,
           bnnn_paging,
           sendSuccessNoIdle(),
           {.title = "Rem. credential",
            .text = (char *)global.signCredentialDeploymentContext.credentialId});
UX_FLOW(ux_sign_credential_update_id, &ux_sign_credential_update_id_0_step);

UX_STEP_NOCB(ux_sign_credential_update_threshold_0_step,
             bnnn_paging,
             {.title = "Cred. sig. threshold",
              .text = (char *)global.signCredentialDeploymentContext.threshold});
UX_STEP_CB(ux_sign_credential_update_threshold_1_step,
           pnn,
           buildAndSignTransactionHash(),
           {&C_icon_validate_14, "Sign", "transaction"});
UX_STEP_CB(ux_sign_credential_update_threshold_2_step,
           pnn,
           sendUserRejection(),
           {&C_icon_crossmark, "Decline to", "sign transaction"});
UX_FLOW(ux_sign_credential_update_threshold,
        &ux_sign_credential_update_threshold_0_step,
        &ux_sign_credential_update_threshold_1_step,
        &ux_sign_credential_update_threshold_2_step);

void uiSignUpdateCredentialInitialDisplay(volatile unsigned int *flags) {
    ux_flow_init(0, ux_update_credentials_initial_flow, NULL);
    *flags |= IO_ASYNCH_REPLY;
}

void uiSignUpdateCredentialIdDisplay(volatile unsigned int *flags) {
    ux_flow_init(0, ux_sign_credential_update_id, NULL);
    *flags |= IO_ASYNCH_REPLY;
}

void uiSignUpdateCredentialThresholdDisplay(volatile unsigned int *flags) {
    ux_flow_init(0, ux_sign_credential_update_threshold, NULL);
    *flags |= IO_ASYNCH_REPLY;
}

void uiSignCredentialDeploymentVerificationKeyDisplay(volatile unsigned int *flags) {
    ux_flow_init(0, ux_credential_deployment_verification_key_flow_with_intro, NULL);
    *flags |= IO_ASYNCH_REPLY;
}

void uiSignCredentialDeploymentVerificationKeyFlowDisplay(volatile unsigned int *flags) {
    ux_flow_init(0, ux_credential_deployment_verification_key_flow, NULL);
    *flags |= IO_ASYNCH_REPLY;
}

void uiSignCredentialDeploymentNewIntroDisplay(void) {
    ux_flow_init(0, ux_sign_credential_deployment_new_with_intro, NULL);
}

void uiSignCredentialDeploymentNewDisplay(void) {
    ux_flow_init(0, ux_sign_credential_deployment_new, NULL);
}

void uiSignCredentialDeploymentExistingIntroDisplay(void) {
    ux_flow_init(0, ux_sign_credential_deployment_existing_with_intro, NULL);
}

void uiSignCredentialDeploymentExistingDisplay(void) {
    ux_flow_init(0, ux_sign_credential_deployment_existing, NULL);
}

// Public information for IP

UX_STEP_NOCB(ux_sign_public_info_for_ip_display_public_key,
             bnnn_paging,
             {.title = "Public key", .text = (char *)global.signPublicInformationForIp.publicKey});

UX_STEP_NOCB(ux_sign_public_info_for_ip_display_key_type,
             bnnn_paging,
             {.title = "Key type", .text = (char *)global.signPublicInformationForIp.keyType});

UX_STEP_NOCB(ux_sign_public_info_for_ip_display_id_cred_pub,
             bnnn_paging,
             {.title = "Id Cred Pub", .text = (char *)global.signPublicInformationForIp.idCredPub});

UX_STEP_NOCB(ux_sign_public_info_for_ip_display_cred_id,
             bnnn_paging,
             {.title = "Credential ID", .text = (char *)global.signPublicInformationForIp.credId});

UX_STEP_CB(ux_sign_public_info_for_ip_continue,
           nn,
           sendSuccessNoIdle(),
           {"Continue", "reviewing info"});

UX_STEP_CB(ux_sign_public_info_review,
           nn,
           sendSuccessNoIdle(),
           {"Review identity", "provider info"});

UX_STEP_CB(ux_sign_public_info_for_ip_sign,
           pnn,
           buildAndSignTransactionHash(),
           {&C_icon_validate_14, "Sign identity", "provider info"});

UX_STEP_CB(ux_sign_public_info_for_ip_decline,
           pnn,
           sendUserRejection(),
           {&C_icon_crossmark, "Decline to", "sign info"});

UX_STEP_NOCB(ux_sign_public_info_for_ip_display_threshold,
             bn,
             {"Signature threshold", (char *)global.signPublicInformationForIp.threshold});

// Display a public key with continue
UX_FLOW(ux_sign_public_info_for_ip_public_key,
        &ux_sign_public_info_for_ip_display_public_key,
        &ux_sign_public_info_for_ip_display_key_type,

        &ux_sign_public_info_for_ip_continue);
// Display intro view and a public key with continue
UX_FLOW(ux_review_public_info_for_ip,
        &ux_sign_public_info_review,
        &ux_sign_public_info_for_ip_display_public_key,
        &ux_sign_public_info_for_ip_display_key_type,
        &ux_sign_public_info_for_ip_display_id_cred_pub,
        &ux_sign_public_info_for_ip_display_cred_id,
        &ux_sign_public_info_for_ip_continue);
// Display last public key and threshold and respond with signature / rejection
UX_FLOW(ux_sign_public_info_for_ip_final,
        &ux_sign_public_info_for_ip_display_public_key,
        &ux_sign_public_info_for_ip_display_key_type,
        &ux_sign_public_info_for_ip_display_threshold,
        &ux_sign_public_info_for_ip_sign,
        &ux_sign_public_info_for_ip_decline);
// Display entire flow and respond with signature / rejection
UX_FLOW(ux_sign_public_info_for_ip_complete,
        &ux_sign_public_info_review,
        &ux_sign_public_info_for_ip_display_public_key,
        &ux_sign_public_info_for_ip_display_key_type,
        &ux_sign_public_info_for_ip_display_id_cred_pub,
        &ux_sign_public_info_for_ip_display_cred_id,
        &ux_sign_public_info_for_ip_display_threshold,
        &ux_sign_public_info_for_ip_sign,
        &ux_sign_public_info_for_ip_decline);

void uiReviewPublicInformationForIpDisplay(void) {
    ux_flow_init(0, ux_review_public_info_for_ip, NULL);
}

void uiSignPublicInformationForIpPublicKeyDisplay(void) {
    ux_flow_init(0, ux_sign_public_info_for_ip_public_key, NULL);
}

void uiSignPublicInformationForIpCompleteDisplay(void) {
    ux_flow_init(0, ux_sign_public_info_for_ip_complete, NULL);
}

void uiSignPublicInformationForIpFinalDisplay(void) {
    ux_flow_init(0, ux_sign_public_info_for_ip_final, NULL);
}

// Register data

UX_STEP_VALID(ux_register_data_initial_flow_step,
              nn,
              sendSuccessNoIdle(),
              {"Continue", "with transaction"});
UX_STEP_VALID(ux_register_data_display_data_step,
              bnnn_paging,
              handleData(),
              {"Data", (char *)global.withDataBlob.cborContext.display});
UX_FLOW(ux_register_data_initial,
        &ux_sign_flow_shared_review,
        &ux_sign_flow_account_sender_view,
        &ux_register_data_initial_flow_step);

UX_FLOW(ux_register_data_payload, &ux_register_data_display_data_step);

void uiSignFlowSharedDisplay(void) {
    ux_flow_init(0, ux_sign_flow_shared, NULL);
}

void uiRegisterDataInitialDisplay(volatile unsigned int *flags) {
    ux_flow_init(0, ux_register_data_initial, NULL);
    *flags |= IO_ASYNCH_REPLY;
}

void uiRegisterDataPayloadDisplay(volatile unsigned int *flags) {
    ux_flow_init(0, ux_register_data_payload, NULL);
    *flags |= IO_ASYNCH_REPLY;
}

// Sign Transfer
const ux_flow_step_t *ux_sign_amount_transfer[8];

UX_STEP_NOCB(ux_sign_flow_1_step,
             bnnn_paging,
             {"Amount", (char *)global.withDataBlob.signTransferContext.displayAmount});

UX_STEP_NOCB(ux_sign_flow_2_step,
             bnnn_paging,
             {.title = "Recipient",
              .text = (char *)global.withDataBlob.signTransferContext.displayStr});

void startTransferDisplay(bool displayMemo, volatile unsigned int *flags) {
    uint8_t index = 0;

    ux_sign_amount_transfer[index++] = &ux_sign_flow_shared_review;
    ux_sign_amount_transfer[index++] = &ux_sign_flow_account_sender_view;
    ux_sign_amount_transfer[index++] = &ux_sign_flow_1_step;
    ux_sign_amount_transfer[index++] = &ux_sign_flow_2_step;

    if (displayMemo) {
        ux_sign_amount_transfer[index++] = &ux_display_memo_step_nocb;
    }

    ux_sign_amount_transfer[index++] = &ux_sign_flow_shared_sign;
    ux_sign_amount_transfer[index++] = &ux_sign_flow_shared_decline;

    ux_sign_amount_transfer[index++] = FLOW_END_STEP;
    ux_flow_init(0, ux_sign_amount_transfer, NULL);
    *flags |= IO_ASYNCH_REPLY;
}

// Sign Transfer to Public

UX_STEP_NOCB(ux_sign_transfer_to_public_1_step,
             bnnn_paging,
             {.title = "Unshield amount", .text = (char *)global.signTransferToPublic.amount});
UX_STEP_NOCB(ux_sign_transfer_to_public_2_step,
             bnnn_paging,
             {.title = "Recipient", .text = (char *)global.signTransferToPublic.recipientAddress});
UX_FLOW(ux_sign_transfer_to_public,
        &ux_sign_flow_shared_review,
        &ux_sign_flow_account_sender_view,
        &ux_sign_transfer_to_public_1_step,
        &ux_sign_transfer_to_public_2_step,
        &ux_sign_flow_shared_sign,
        &ux_sign_flow_shared_decline);

void uiSignTransferToPublicDisplay(volatile unsigned int *flags) {
    ux_flow_init(0, ux_sign_transfer_to_public, NULL);
    *flags |= IO_ASYNCH_REPLY;
}

// Sign Transfer with Schedule
const ux_flow_step_t *ux_sign_scheduled_amount_transfer[8];
static signTransferWithScheduleContext_t *ctx_sign_transfer_with_schedule =
    &global.withDataBlob.signTransferWithScheduleContext;

// UI definitions for displaying the transaction contents of the first packet for verification
// before continuing to process the scheduled amount pairs that will be received in separate
// packets.
UX_STEP_NOCB(ux_scheduled_transfer_initial_flow_1_step,
             bnnn_paging,
             {.title = "Recipient",
              .text = (char *)global.withDataBlob.signTransferWithScheduleContext.displayStr});
UX_STEP_VALID(ux_scheduled_transfer_initial_flow_2_step,
              nn,
              sendSuccessNoIdle(),
              {"Continue", "with transaction"});

// UI definitions for displaying a timestamp and an amount of a scheduled transfer.
UX_STEP_NOCB(ux_sign_scheduled_transfer_pair_flow_0_step,
             bnnn_paging,
             {"Release time (UTC)",
              (char *)global.withDataBlob.signTransferWithScheduleContext.displayTimestamp});
UX_STEP_NOCB(ux_sign_scheduled_transfer_pair_flow_1_step,
             bnnn_paging,
             {"Amount", (char *)global.withDataBlob.signTransferWithScheduleContext.displayAmount});
UX_STEP_CB(ux_sign_scheduled_transfer_pair_flow_2_step,
           nn,
           processNextScheduledAmount(ctx_sign_transfer_with_schedule->buffer),
           {"Show", "next release"});
UX_FLOW(ux_sign_scheduled_transfer_pair_flow,
        &ux_sign_scheduled_transfer_pair_flow_0_step,
        &ux_sign_scheduled_transfer_pair_flow_1_step,
        &ux_sign_scheduled_transfer_pair_flow_2_step);

UX_FLOW(ux_sign_scheduled_transfer_pair_flow_sign,
        &ux_sign_scheduled_transfer_pair_flow_0_step,
        &ux_sign_scheduled_transfer_pair_flow_1_step,
        &ux_sign_flow_shared_sign,
        &ux_sign_flow_shared_decline);

void startInitialScheduledTransferDisplay(bool displayMemo) {
    uint8_t index = 0;

    ux_sign_scheduled_amount_transfer[index++] = &ux_sign_flow_shared_review;
    ux_sign_scheduled_amount_transfer[index++] = &ux_sign_flow_account_sender_view;
    ux_sign_scheduled_amount_transfer[index++] = &ux_scheduled_transfer_initial_flow_1_step;

    if (displayMemo) {
        ux_sign_scheduled_amount_transfer[index++] = &ux_display_memo_step_nocb;
    }

    ux_sign_scheduled_amount_transfer[index++] = &ux_scheduled_transfer_initial_flow_2_step;

    ux_sign_scheduled_amount_transfer[index++] = FLOW_END_STEP;
    ux_flow_init(0, ux_sign_scheduled_amount_transfer, NULL);
}

void uiSignScheduledTransferPairFlowSignDisplay(void) {
    ux_flow_init(0, ux_sign_scheduled_transfer_pair_flow_sign, NULL);
}

void uiSignScheduledTransferPairFlowDisplay(void) {
    ux_flow_init(0, ux_sign_scheduled_transfer_pair_flow, NULL);
}

// Deploy Module
UX_STEP_NOCB(ux_deploy_module_1_step,
             bnnn_paging,
             {.title = "Version", .text = (char *)global.deployModule.versionDisplay});
UX_STEP_NOCB(ux_deploy_module_2_step,
             bnnn_paging,
             {.title = "TX hash", .text = (char *)global.deployModule.sourceHashDisplay});
UX_FLOW(ux_deploy_module,
        &ux_sign_flow_shared_review,
        &ux_sign_flow_account_sender_view,
        &ux_deploy_module_1_step,
        // &ux_deploy_module_2_step,
        &ux_sign_flow_shared_sign,
        &ux_sign_flow_shared_decline);

void uiDeployModuleDisplay() {
    ux_flow_init(0, ux_deploy_module, NULL);
}

// Init Contract
UX_STEP_NOCB(ux_init_contract_1_step,
             bnnn_paging,
             {.title = "Amount", .text = (char *)global.initContract.amountDisplay});
UX_STEP_NOCB(ux_init_contract_2_step,
             bnnn_paging,
             {.title = "Module ref", .text = (char *)global.initContract.moduleRefDisplay});
UX_FLOW(ux_init_contract,
        &ux_sign_flow_shared_review,
        &ux_sign_flow_account_sender_view,
        &ux_init_contract_1_step,
        &ux_init_contract_2_step,
        &ux_sign_flow_shared_sign,
        &ux_sign_flow_shared_decline);

void uiInitContractDisplay() {
    ux_flow_init(0, ux_init_contract, NULL);
}

// Update Contract
UX_STEP_NOCB(ux_update_contract_1_step,
             bnnn_paging,
             {.title = "Amount", .text = (char *)global.updateContract.amountDisplay});
UX_STEP_NOCB(ux_update_contract_2_step,
             bnnn_paging,
             {.title = "Index", .text = (char *)global.updateContract.indexDisplay});
UX_STEP_NOCB(ux_update_contract_3_step,
             bnnn_paging,
             {.title = "Sub index", .text = (char *)global.updateContract.subIndexDisplay});
UX_FLOW(ux_update_contract,
        &ux_sign_flow_shared_review,
        &ux_sign_flow_account_sender_view,
        &ux_update_contract_1_step,
        &ux_update_contract_2_step,
        &ux_update_contract_3_step,
        &ux_sign_flow_shared_sign,
        &ux_sign_flow_shared_decline);

void uiUpdateContractDisplay() {
    ux_flow_init(0, ux_update_contract, NULL);
}

#endif
