#include <os.h>

#include "common/ui/display.h"
#include "responseCodes.h"
#include "sign.h"
#include "util.h"
#include "signConfigureBaker.h"

static signConfigureBaker_t *ctx_conf_baker = &global.signConfigureBaker;
static tx_state_t *tx_state = &global_tx_state;

bool hasCommissionRate() {
    return ctx_conf_baker->hasTransactionFeeCommission ||
           ctx_conf_baker->hasBakingRewardCommission ||
           ctx_conf_baker->hasFinalizationRewardCommission;
}

#define P1_INITIAL          0x00
#define P1_FIRST_BATCH      0x01
#define P1_AGGREGATION_KEY  0x02
#define P1_URL_LENGTH       0x03
#define P1_URL              0x04
#define P1_COMMISSION_RATES 0x05

void handleCommissionRates(uint8_t *cdata, uint8_t dataLength) {
    if (ctx_conf_baker->hasTransactionFeeCommission) {
        uint32_t rate = U4BE(cdata, 0);
        fractionToPercentageDisplay(
            ctx_conf_baker->commissionRates.transactionFeeCommissionRate,
            sizeof(ctx_conf_baker->commissionRates.transactionFeeCommissionRate),
            rate);
        updateHash((cx_hash_t *) &tx_state->hash, cdata, 4);
        cdata += 4;
        dataLength -= 4;
    }

    if (ctx_conf_baker->hasBakingRewardCommission) {
        uint32_t rate = U4BE(cdata, 0);
        fractionToPercentageDisplay(
            ctx_conf_baker->commissionRates.bakingRewardCommissionRate,
            sizeof(ctx_conf_baker->commissionRates.bakingRewardCommissionRate),
            rate);
        updateHash((cx_hash_t *) &tx_state->hash, cdata, 4);
        cdata += 4;
        dataLength -= 4;
    }

    if (ctx_conf_baker->hasFinalizationRewardCommission) {
        uint32_t rate = U4BE(cdata, 0);
        fractionToPercentageDisplay(
            ctx_conf_baker->commissionRates.finalizationRewardCommissionRate,
            sizeof(ctx_conf_baker->commissionRates.finalizationRewardCommissionRate),
            rate);
        updateHash((cx_hash_t *) &tx_state->hash, cdata, 4);
        dataLength -= 4;
    }

    if (dataLength != 0) {
        THROW(ERROR_INVALID_TRANSACTION);
    }

    startConfigureBakerCommissionDisplay();
}

void handleSignConfigureBaker(uint8_t *cdata,
                              uint8_t p1,
                              uint8_t dataLength,
                              volatile unsigned int *flags,
                              bool isInitialCall) {
    if (P1_INITIAL == p1 && isInitialCall) {
        cdata += parseKeyDerivationPath(cdata);
        cx_sha256_init(&tx_state->hash);
        cdata += hashAccountTransactionHeaderAndKind(cdata, CONFIGURE_BAKER);
        ctx_conf_baker->firstDisplay = true;

        // The initial 2 bytes tells us the fields we are receiving.
        updateHash((cx_hash_t *) &tx_state->hash, cdata, 2);
        uint16_t bitmap = U2BE(cdata, 0);

        // An empty transaction with none of the optionals available is invalid,
        // or any transaction with a bit set after the 10th bits place (as there are 10
        // optionals).
        if (bitmap == 0 || bitmap > 1023) {
            THROW(ERROR_INVALID_TRANSACTION);
        }

        ctx_conf_baker->hasCapital = (bitmap >> 0) & 1;
        ctx_conf_baker->hasRestakeEarnings = (bitmap >> 1) & 1;
        ctx_conf_baker->hasOpenForDelegation = (bitmap >> 2) & 1;
        ctx_conf_baker->hasKeys = (bitmap >> 3) & 1;
        ctx_conf_baker->hasMetadataUrl = (bitmap >> 4) & 1;
        ctx_conf_baker->hasTransactionFeeCommission = (bitmap >> 5) & 1;
        ctx_conf_baker->hasBakingRewardCommission = (bitmap >> 6) & 1;
        ctx_conf_baker->hasFinalizationRewardCommission = (bitmap >> 7) & 1;

        if (ctx_conf_baker->hasCapital || ctx_conf_baker->hasRestakeEarnings ||
            ctx_conf_baker->hasOpenForDelegation || ctx_conf_baker->hasKeys) {
            ctx_conf_baker->state = CONFIGURE_BAKER_FIRST;
        } else if (ctx_conf_baker->hasMetadataUrl) {
            ctx_conf_baker->state = CONFIGURE_BAKER_URL_LENGTH;
        } else if (hasCommissionRate()) {
            ctx_conf_baker->state = CONFIGURE_BAKER_COMMISSION_RATES;
        }

        sendSuccessNoIdle();
    } else if (P1_FIRST_BATCH == p1 && ctx_conf_baker->state == CONFIGURE_BAKER_FIRST) {
        int lengthCheck = dataLength;

        if (ctx_conf_baker->hasCapital) {
            uint64_t capitalAmount = U8BE(cdata, 0);
            if (capitalAmount == 0) {
                ctx_conf_baker->capitalRestakeDelegation.stopBaking = true;
            } else {
                ctx_conf_baker->capitalRestakeDelegation.stopBaking = false;
                amountToGtuDisplay(ctx_conf_baker->capitalRestakeDelegation.displayCapital,
                                   sizeof(ctx_conf_baker->capitalRestakeDelegation.displayCapital),
                                   capitalAmount);
            }
            updateHash((cx_hash_t *) &tx_state->hash, cdata, 8);
            cdata += 8;
            lengthCheck -= 8;
        }

        if (ctx_conf_baker->hasRestakeEarnings) {
            uint8_t restake = cdata[0];
            updateHash((cx_hash_t *) &tx_state->hash, cdata, 1);
            cdata += 1;
            lengthCheck -= 1;
            if (restake == 0) {
                memmove(ctx_conf_baker->capitalRestakeDelegation.displayRestake, "No", 3);
            } else if (restake == 1) {
                memmove(ctx_conf_baker->capitalRestakeDelegation.displayRestake, "Yes", 4);
            } else {
                THROW(ERROR_INVALID_TRANSACTION);
            }
        }

        if (ctx_conf_baker->hasOpenForDelegation) {
            uint8_t openForDelegation = cdata[0];
            updateHash((cx_hash_t *) &tx_state->hash, cdata, 1);
            cdata += 1;
            lengthCheck -= 1;

            if (openForDelegation == 0) {
                memmove(ctx_conf_baker->capitalRestakeDelegation.displayOpenForDelegation,
                        "Open for all",
                        13);
            } else if (openForDelegation == 1) {
                memmove(ctx_conf_baker->capitalRestakeDelegation.displayOpenForDelegation,
                        "Closed for new",
                        15);
            } else if (openForDelegation == 2) {
                memmove(ctx_conf_baker->capitalRestakeDelegation.displayOpenForDelegation,
                        "Closed for all",
                        15);
            } else {
                THROW(ERROR_INVALID_TRANSACTION);
            }
        }

        if (ctx_conf_baker->hasKeys) {
            // We are expecting the signature and election verification keys (each 32 bytes) and
            // their proofs (each 64 bytes).
            if (lengthCheck != 192) {
                THROW(ERROR_INVALID_TRANSACTION);
            }

            // We do not display the verification keys to the user, as they are difficult
            // for the user to verify. If need be, we can start showing them by parsing
            // the values into hex strings here.
            // Election verify key
            updateHash((cx_hash_t *) &tx_state->hash, cdata, 32);
            cdata += 32;

            // Election Proof
            updateHash((cx_hash_t *) &tx_state->hash, cdata, 64);
            cdata += 64;

            // Signature verify key
            updateHash((cx_hash_t *) &tx_state->hash, cdata, 32);
            cdata += 32;

            // Signature Proof
            updateHash((cx_hash_t *) &tx_state->hash, cdata, 64);

            // We delay the display until we get the aggregation key.
            ctx_conf_baker->state = CONFIGURE_BAKER_AGGREGATION_KEY;
            sendSuccessNoIdle();
        } else {
            if (lengthCheck != 0) {
                THROW(ERROR_INVALID_TRANSACTION);
            }

            if (ctx_conf_baker->hasMetadataUrl) {
                ctx_conf_baker->state = CONFIGURE_BAKER_URL_LENGTH;
            } else if (hasCommissionRate()) {
                ctx_conf_baker->state = CONFIGURE_BAKER_COMMISSION_RATES;
            } else {
                ctx_conf_baker->state = CONFIGURE_BAKER_END;
            }

            startConfigureBakerDisplay();
            *flags |= IO_ASYNCH_REPLY;
        }
    } else if (P1_AGGREGATION_KEY == p1 &&
               ctx_conf_baker->state == CONFIGURE_BAKER_AGGREGATION_KEY) {
        if (!ctx_conf_baker->hasKeys || dataLength != 160) {
            THROW(ERROR_INVALID_TRANSACTION);
        }

        // Aggregation verify key
        updateHash((cx_hash_t *) &tx_state->hash, cdata, 96);
        cdata += 96;

        // Election Proof
        updateHash((cx_hash_t *) &tx_state->hash, cdata, 64);

        if (ctx_conf_baker->hasMetadataUrl) {
            ctx_conf_baker->state = CONFIGURE_BAKER_URL_LENGTH;
        } else if (hasCommissionRate()) {
            ctx_conf_baker->state = CONFIGURE_BAKER_COMMISSION_RATES;
        } else {
            ctx_conf_baker->state = CONFIGURE_BAKER_END;
        }

        startConfigureBakerDisplay();
        *flags |= IO_ASYNCH_REPLY;
    } else if (P1_URL_LENGTH == p1 && ctx_conf_baker->state == CONFIGURE_BAKER_URL_LENGTH) {
        if (!ctx_conf_baker->hasMetadataUrl) {
            THROW(ERROR_INVALID_TRANSACTION);
        }

        ctx_conf_baker->url.urlLength = U2BE(cdata, 0);
        if (ctx_conf_baker->url.urlLength > 2048) {
            THROW(ERROR_INVALID_TRANSACTION);
        }

        updateHash((cx_hash_t *) &tx_state->hash, cdata, 2);

        if (ctx_conf_baker->url.urlLength == 0) {
            // If the url has length zero, we don't wait for the url bytes.
            if (hasCommissionRate()) {
                ctx_conf_baker->state = CONFIGURE_BAKER_COMMISSION_RATES;
            } else {
                ctx_conf_baker->state = CONFIGURE_BAKER_END;
            }

            startConfigureBakerUrlDisplay(true);
            *flags |= IO_ASYNCH_REPLY;
        } else {
            ctx_conf_baker->state = CONFIGURE_BAKER_URL;
            sendSuccessNoIdle();
        }
    } else if (P1_URL == p1 && ctx_conf_baker->state == CONFIGURE_BAKER_URL) {
        if (ctx_conf_baker->url.urlLength > dataLength) {
            updateHash((cx_hash_t *) &tx_state->hash, cdata, dataLength);
            ctx_conf_baker->url.urlLength -= dataLength;
            memmove(ctx_conf_baker->url.urlDisplay, cdata, dataLength);
            startConfigureBakerUrlDisplay(false);
            *flags |= IO_ASYNCH_REPLY;
        } else if (ctx_conf_baker->url.urlLength == dataLength) {
            memmove(ctx_conf_baker->url.urlDisplay, cdata, ctx_conf_baker->url.urlLength);
            memmove(ctx_conf_baker->url.urlDisplay + ctx_conf_baker->url.urlLength, "\0", 1);
            updateHash((cx_hash_t *) &tx_state->hash, cdata, ctx_conf_baker->url.urlLength);

            if (hasCommissionRate()) {
                ctx_conf_baker->state = CONFIGURE_BAKER_COMMISSION_RATES;
            } else {
                ctx_conf_baker->state = CONFIGURE_BAKER_END;
            }

            startConfigureBakerUrlDisplay(true);
            *flags |= IO_ASYNCH_REPLY;
        } else {
            THROW(ERROR_INVALID_TRANSACTION);
        }
    } else if (P1_COMMISSION_RATES == p1 &&
               ctx_conf_baker->state == CONFIGURE_BAKER_COMMISSION_RATES) {
        handleCommissionRates(cdata, dataLength);
        *flags |= IO_ASYNCH_REPLY;
    } else {
        THROW(ERROR_INVALID_STATE);
    }
}
