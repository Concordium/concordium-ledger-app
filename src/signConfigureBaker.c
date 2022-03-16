#include <os.h>

#include "accountSenderView.h"
#include "responseCodes.h"
#include "sign.h"
#include "util.h"
static signConfigureBaker_t *ctx = &global.signConfigureBaker;
static tx_state_t *tx_state = &global_tx_state;

// Define the dynamic UI elements. These are required as the majority of
// the transaction elements are optional, so the UI has to be dynamically set.
const ux_flow_step_t *ux_sign_configure_baker_first[8];
const ux_flow_step_t *ux_sign_configure_baker_url[6];
const ux_flow_step_t *ux_sign_configure_baker_commission[8];

UX_STEP_NOCB(
    ux_sign_configure_baker_capital_step,
    bnnn_paging,
    {.title = "Capital", .text = (char *) global.signConfigureBaker.displayCapital});

UX_STEP_NOCB(
    ux_sign_configure_baker_restake_step,
    bnnn_paging,
    {.title = "Restake earnings", .text = (char *) global.signConfigureBaker.displayRestake});

UX_STEP_NOCB(
    ux_sign_configure_baker_open_status_step,
    bnnn_paging,
    {.title = "Open status", .text = (char *) global.signConfigureBaker.displayOpenForDelegation});

UX_STEP_NOCB(ux_sign_configure_baker_keys_step, nn, {"Update baker", "keys"});

UX_STEP_CB(
    ux_sign_configure_baker_url_cb_step,
    bnnn_paging,
    sendSuccessNoIdle(),
    {.title = "URL", .text = (char *) global.signConfigureBaker.url});

UX_STEP_NOCB(
    ux_sign_configure_baker_url_step,
    bnnn_paging,
    {.title = "URL", .text = (char *) global.signConfigureBaker.url});

UX_STEP_CB(ux_sign_configure_baker_continue, nn, sendSuccessNoIdle(), {"Continue", "with transaction"});

UX_STEP_NOCB(
    ux_sign_configure_baker_empty_url_step,
    bn,
    {"Empty URL", ""});

UX_STEP_NOCB(
    ux_sign_configure_baker_commission_transaction_fee_step,
    bnnn_paging,
    {.title = "Transaction fee", .text = (char *) global.signConfigureBaker.transactionFeeCommissionRate});

UX_STEP_NOCB(
    ux_sign_configure_baker_commission_baking_reward_step,
    bnnn_paging,
    {.title = "Baking reward", .text = (char *) global.signConfigureBaker.bakingRewardCommissionRate});

UX_STEP_NOCB(
    ux_sign_configure_baker_commission_finalization_reward_step,
    bnnn_paging,
    {.title = "Finalization reward", .text = (char *) global.signConfigureBaker.finalizationRewardCommissionRate});

/**
 * Dynamically builds and initializes the capital, restake earnings, open status and
 * baker keys display.
 * - Ensures that the UI starts with the shared review transaction screens.
 * - Only displays the parts of the transaction that are set in the transaction, and skips
 *   any optional fields that are not included.
 * - If there are either the URL or commission rates in the transaction, then it shows a continue screen
 *   at the end.
 */
void startConfigureBakerDisplay() {
    uint8_t index = 0;

    ux_sign_configure_baker_first[index++] = &ux_sign_flow_shared_review;
    ux_sign_configure_baker_first[index++] = &ux_sign_flow_account_sender_view;
    ctx->firstDisplay = false;

    if (ctx->hasCapital) {
        ux_sign_configure_baker_first[index++] = &ux_sign_configure_baker_capital_step;
    }

    if (ctx->hasRestakeEarnings) {
        ux_sign_configure_baker_first[index++] = &ux_sign_configure_baker_restake_step;
    }

    if (ctx->hasOpenForDelegation) {
        ux_sign_configure_baker_first[index++] = &ux_sign_configure_baker_open_status_step;
    }

    if (ctx->hasKeys) {
        ux_sign_configure_baker_first[index++] = &ux_sign_configure_baker_keys_step;
    }

    // TODO: Fix display for keys. We probably want to show them so that an update to the
    // keys is shown and does not just look like an empty transaction. Or at least we have
    // to display something stating that the keys are updated.

    // If there are additional steps, then show continue screen. If this is the last step,
    // then show signing screens.
    if (ctx->hasMetadataUrl || ctx->hasTransactionFeeCommission || ctx->hasBakingRewardCommission ||
        ctx->hasFinalizationRewardCommission) {
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
 * - If it is the final part of the URL display, then the URL screen does not have a callback to continue
 *   as additional UI elements are added to guide the user forward.
 */
void startConfigureBakerUrlDisplay(bool lastUrlPage) {
    uint8_t index = 0;

    if (ctx->firstDisplay) {
        ux_sign_configure_baker_url[index++] = &ux_sign_flow_shared_review;
        ux_sign_configure_baker_url[index++] = &ux_sign_flow_account_sender_view;
        ctx->firstDisplay = false;
    }

    if (!lastUrlPage) {
        ux_sign_configure_baker_url[index++] = &ux_sign_configure_baker_url_cb_step;
    } else {

        if (ctx->urlLength == 0) {
            ux_sign_configure_baker_url[index++] = &ux_sign_configure_baker_empty_url_step;
        } else {
            ux_sign_configure_baker_url[index++] = &ux_sign_configure_baker_url_step;
        }

        // If there are additional steps show the continue screen, otherwise go
        // to signing screens.
        if (ctx->hasTransactionFeeCommission || ctx->hasBakingRewardCommission ||
            ctx->hasFinalizationRewardCommission) {
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

    if (ctx->firstDisplay) {
        ux_sign_configure_baker_commission[index++] = &ux_sign_flow_shared_review;
        ux_sign_configure_baker_commission[index++] = &ux_sign_flow_account_sender_view;
        ctx->firstDisplay = false;
    }

    if (ctx->hasTransactionFeeCommission) {
        ux_sign_configure_baker_commission[index++] = &ux_sign_configure_baker_commission_transaction_fee_step;
    }

    if (ctx->hasBakingRewardCommission) {
        ux_sign_configure_baker_commission[index++] = &ux_sign_configure_baker_commission_baking_reward_step;
    }

    if (ctx->hasFinalizationRewardCommission) {
        ux_sign_configure_baker_commission[index++] = &ux_sign_configure_baker_commission_finalization_reward_step;
    }

    ux_sign_configure_baker_commission[index++] = &ux_sign_flow_shared_sign;
    ux_sign_configure_baker_commission[index++] = &ux_sign_flow_shared_decline;

    ux_sign_configure_baker_commission[index++] = FLOW_END_STEP;
    ux_flow_init(0, ux_sign_configure_baker_commission, NULL);
}

// TODO This methods could/should be shared with what we use for the other reward fractions.
/**
 * Helper method for parsing commission rates as they are all equal in structure.
 */
uint8_t parseCommissionRate(uint8_t *cdata, uint8_t *commissionRateDisplay, uint8_t sizeOfCommissionRateDisplay) {
    uint8_t fraction[8] = "/100000";

    uint32_t rate = U4BE(cdata, 0);
    int rateLength = numberToText(commissionRateDisplay, sizeOfCommissionRateDisplay, rate);
    cx_hash((cx_hash_t *) &tx_state->hash, 0, cdata, 4, NULL, 0);
    memmove(commissionRateDisplay + rateLength, fraction, 8);
    return 4;
}

#define P1_INITIAL          0x00
#define P1_FIRST_BATCH      0x01
#define P1_AGGREGATION_KEY  0x02
#define P1_URL_LENGTH       0x03
#define P1_URL              0x04
#define P1_COMMISSION_RATES 0x05

void handleCommissionRates(uint8_t *cdata, uint8_t dataLength) {
    uint8_t rateLength;

    if (ctx->hasTransactionFeeCommission) {
        rateLength =
            parseCommissionRate(cdata, ctx->transactionFeeCommissionRate, sizeof(ctx->transactionFeeCommissionRate));
        cdata += rateLength;
        dataLength -= rateLength;
    }

    if (ctx->hasBakingRewardCommission) {
        rateLength =
            parseCommissionRate(cdata, ctx->bakingRewardCommissionRate, sizeof(ctx->bakingRewardCommissionRate));
        cdata += rateLength;
        dataLength -= rateLength;
    }

    if (ctx->hasFinalizationRewardCommission) {
        rateLength = parseCommissionRate(
            cdata,
            ctx->finalizationRewardCommissionRate,
            sizeof(ctx->finalizationRewardCommissionRate));
        dataLength -= rateLength;
    }

    if (dataLength != 0) {
        THROW(ERROR_INVALID_TRANSACTION);
    }

    startConfigureBakerCommissionDisplay();
}

void handleSignConfigureBaker(
    uint8_t *cdata,
    uint8_t p1,
    uint8_t dataLength,
    volatile unsigned int *flags,
    bool isInitialCall) {
    if (P1_INITIAL == p1 && isInitialCall) {
        cdata += parseKeyDerivationPath(cdata);
        cx_sha256_init(&tx_state->hash);
        cdata += hashAccountTransactionHeaderAndKind(cdata, CONFIGURE_BAKER);
        ctx->firstDisplay = true;

        // The initial 2 bytes tells us the fields we are receiving.
        cx_hash((cx_hash_t *) &tx_state->hash, 0, cdata, 2, NULL, 0);
        uint16_t bitmap = U2BE(cdata, 0);

        // An empty transaction with none of the optionals available is invalid,
        // or any transaction with a bit set after the 10th bits place (as there are 10
        // optionals).
        if (bitmap == 0 || bitmap > 1023) {
            THROW(ERROR_INVALID_TRANSACTION);
        }

        ctx->hasCapital = (bitmap >> 0) & 1;
        ctx->hasRestakeEarnings = (bitmap >> 1) & 1;
        ctx->hasOpenForDelegation = (bitmap >> 2) & 1;
        ctx->hasKeys = (bitmap >> 3) & 1;
        ctx->hasMetadataUrl = (bitmap >> 4) & 1;
        ctx->hasTransactionFeeCommission = (bitmap >> 5) & 1;
        ctx->hasBakingRewardCommission = (bitmap >> 6) & 1;
        ctx->hasFinalizationRewardCommission = (bitmap >> 7) & 1;

        // TODO: Determine state based on the above booleans. If any in the first
        // section, then expect that part and so on...
        sendSuccessNoIdle();
    } else if (P1_FIRST_BATCH == p1) {
        int lengthCheck = dataLength;

        if (ctx->hasCapital) {
            uint64_t capitalAmount = U8BE(cdata, 0);
            amountToGtuDisplay(ctx->displayCapital, sizeof(ctx->displayCapital), capitalAmount);
            cx_hash((cx_hash_t *) &tx_state->hash, 0, cdata, 8, NULL, 0);
            cdata += 8;
            lengthCheck -= 8;
        }

        if (ctx->hasRestakeEarnings) {
            uint8_t restake = cdata[0];
            cx_hash((cx_hash_t *) &tx_state->hash, 0, cdata, 1, NULL, 0);
            cdata += 1;
            lengthCheck -= 1;
            if (restake == 0) {
                memmove(ctx->displayRestake, "No", 3);
            } else if (restake == 1) {
                memmove(ctx->displayRestake, "Yes", 4);
            } else {
                THROW(ERROR_INVALID_TRANSACTION);
            }
        }

        if (ctx->hasOpenForDelegation) {
            uint8_t openForDelegation = cdata[0];
            cx_hash((cx_hash_t *) &tx_state->hash, 0, cdata, 1, NULL, 0);
            cdata += 1;
            lengthCheck -= 1;

            if (openForDelegation == 0) {
                memmove(ctx->displayOpenForDelegation, "Open for all", 13);
            } else if (openForDelegation == 1) {
                memmove(ctx->displayOpenForDelegation, "Closed for new", 15);
            } else if (openForDelegation == 2) {
                memmove(ctx->displayOpenForDelegation, "Closed for all", 15);
            } else {
                THROW(ERROR_INVALID_TRANSACTION);
            }
        }

        if (ctx->hasKeys) {
            // We are expecting the signature and election verification keys (each 32 bytes) and their proofs (each 64
            // bytes).
            if (lengthCheck != 192) {
                THROW(ERROR_INVALID_TRANSACTION);
            }

            // We do not display the verification keys to the user, as they are difficult
            // for the user to verify. If need be, we can start showing them by parsing
            // the values into hex strings here.
            // Election verify key
            cx_hash((cx_hash_t *) &tx_state->hash, 0, cdata, 32, NULL, 0);
            cdata += 32;

            // Election Proof
            cx_hash((cx_hash_t *) &tx_state->hash, 0, cdata, 64, NULL, 0);
            cdata += 64;

            // Signature verify key
            cx_hash((cx_hash_t *) &tx_state->hash, 0, cdata, 32, NULL, 0);
            cdata += 32;

            // Signature Proof
            cx_hash((cx_hash_t *) &tx_state->hash, 0, cdata, 64, NULL, 0);

            // We delay the display until we get the aggregation key.
            sendSuccessNoIdle();
        } else {
            if (lengthCheck != 0) {
                THROW(ERROR_INVALID_TRANSACTION);
            }
            startConfigureBakerDisplay();
            *flags |= IO_ASYNCH_REPLY;
        }
    } else if (P1_AGGREGATION_KEY == p1) {
        if (!ctx->hasKeys || dataLength != 160) {
            THROW(ERROR_INVALID_TRANSACTION);
        }

        // Aggregation verify key
        cx_hash((cx_hash_t *) &tx_state->hash, 0, cdata, 96, NULL, 0);
        cdata += 96;

        // Election Proof
        cx_hash((cx_hash_t *) &tx_state->hash, 0, cdata, 64, NULL, 0);

        startConfigureBakerDisplay();
        *flags |= IO_ASYNCH_REPLY;
    } else if (P1_URL_LENGTH == p1) {
        if (!ctx->hasMetadataUrl) {
            THROW(ERROR_INVALID_TRANSACTION);
        }

        ctx->urlLength = U2BE(cdata, 0);
        if (ctx->urlLength > 2048) {
            THROW(ERROR_INVALID_TRANSACTION);
        }

        cx_hash((cx_hash_t *) &tx_state->hash, 0, cdata, 2, NULL, 0);

        if (ctx->urlLength == 0) {
            // If the url has length zero, we don't wait for the url bytes.
            startConfigureBakerUrlDisplay(true);
            *flags |= IO_ASYNCH_REPLY;
        } else {
            sendSuccessNoIdle();
        }
    } else if (P1_URL == p1) {
        // NOTE: We don't have to check the bool here, as the state is checked and
        // can only be correct if the bool was set! Nifty!

        // We cannot display strings that are so long... So we actually have to display
        // one at a time then...
        if (ctx->urlLength > dataLength) {
            cx_hash((cx_hash_t *) &tx_state->hash, 0, cdata, dataLength, NULL, 0);
            ctx->urlLength -= dataLength;
            memmove(ctx->url, cdata, dataLength);
            startConfigureBakerUrlDisplay(false);
            *flags |= IO_ASYNCH_REPLY;
        } else if (ctx->urlLength == dataLength) {
            memmove(ctx->url, cdata, ctx->urlLength);
            memmove(ctx->url + ctx->urlLength, "\0", 1);
            cx_hash((cx_hash_t *) &tx_state->hash, 0, cdata, ctx->urlLength, NULL, 0);
            startConfigureBakerUrlDisplay(true);
            *flags |= IO_ASYNCH_REPLY;
        } else {
            THROW(ERROR_INVALID_TRANSACTION);
        }
    } else if (P1_COMMISSION_RATES == p1) {
        handleCommissionRates(cdata, dataLength);
        *flags |= IO_ASYNCH_REPLY;
    } else {
        THROW(ERROR_INVALID_STATE);
    }
}
