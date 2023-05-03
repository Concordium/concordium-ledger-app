#include <os.h>
#include <stdint.h>
#include <string.h>

#include "cx.h"
#include "globals.h"
#include "responseCodes.h"
#include "util.h"
#include "ux.h"

// This class allows for the export of a number of very specific private keys. These private keys are made
// exportable as they are used in computations that are not feasible to carry out on the Ledger device.
// The key derivation paths that are allowed are restricted so that it is not possible to export
// keys that are used for signing.
static const uint32_t HARDENED_OFFSET = 0x80000000;
static exportDataContext_t *ctx = &global.exportDataContext;

void sendDataResponse(bool final);

UX_STEP_NOCB(
    ux_export_data_identity_step,
    nn,
    {"Identity", (char*) global.exportDataContext.identityDisplay});
UX_STEP_NOCB(
    ux_export_data_attribute_step,
    nn,
    {"Export private", "attribute data"});
UX_STEP_NOCB(
    ux_export_data_identity_data_step,
    nn,
    {"Export private", "identity data"});

UX_STEP_CB(ux_export_data_accept_step, pb, sendDataResponse(false), {&C_icon_validate_14, "Accept"});
UX_STEP_CB(ux_export_data_accept_step_final, pb, sendDataResponse(true), {&C_icon_validate_14, "Accept"});
UX_STEP_CB(ux_export_data_decline_step, pb, sendUserRejection(), {&C_icon_crossmark, "Decline"});
UX_FLOW(ux_export_data_only_attribute, &ux_export_data_attribute_step, &ux_export_data_identity_step, &ux_export_data_accept_step, &ux_export_data_decline_step);
UX_FLOW(ux_export_data_no_attribute, &ux_export_data_identity_data_step, &ux_export_data_identity_step, &ux_export_data_accept_step_final, &ux_export_data_decline_step);
UX_FLOW(ux_export_data_all, &ux_export_data_identity_data_step, &ux_export_data_identity_step, &ux_export_data_accept_step, &ux_export_data_decline_step);

void sendDataResponse(bool final) {
    BEGIN_TRY {
        TRY {
            memmove(G_io_apdu_buffer, ctx->output, ctx->outputLength);
            if (final) {
                sendSuccess(ctx->outputLength);
            } else {
                sendSuccessResultNoIdle(ctx->outputLength);
            }
        }
        FINALLY {
            explicit_bzero(ctx->output, ctx->outputLength);
        }
    }
    END_TRY;
}

#define ID_CRED_SEC   0x02
#define PRF_KEY       0x03
#define BLINDING_RANDOMNESS  0x04
#define ATTRIBUTE_RANDOMNESS 0x05

void getAttributeRandomness(uint32_t tag, uint8_t *dst, size_t dstLength) {
    uint32_t path[7] = {
        CONCORDIUM_PURPOSE | HARDENED_OFFSET,
        ctx->coinType | HARDENED_OFFSET,
        ctx->identityProvider | HARDENED_OFFSET,
        ctx->identity | HARDENED_OFFSET,
        ATTRIBUTE_RANDOMNESS | HARDENED_OFFSET,
        ctx->credCounter | HARDENED_OFFSET,
        tag | HARDENED_OFFSET,
    };
    getBlsPrivateKey(path, 7, dst, dstLength);
}

/**
 * Get any identity data that is not attribute randomness.
 */
void getIdentityData(uint32_t type, uint8_t *dst, size_t dstLength) {
    uint32_t path[5] = {
        CONCORDIUM_PURPOSE | HARDENED_OFFSET,
        ctx->coinType | HARDENED_OFFSET,
        ctx->identityProvider | HARDENED_OFFSET,
        ctx->identity | HARDENED_OFFSET,
        type | HARDENED_OFFSET,
    };
    getBlsPrivateKey(path, 5, dst, dstLength);
}

#define P1_INITIAL_NO_ATTRIBUTES     0x00
#define P1_INITIAL_ONLY_ATTRIBUTES     0x01
#define P1_INITIAL_ALL     0x02
#define P1_ATTRIBUTES     0x03

void handleExportPrivateIdentityData(uint8_t *dataBuffer, uint8_t p1, uint8_t dataLength, volatile unsigned int *flags, bool isInitialCall) {
    if (isInitialCall) {
        ctx->state = EXPORT_DATA_INITIAL;
    }

    if ((p1 == P1_INITIAL_ONLY_ATTRIBUTES || p1 == P1_INITIAL_ALL || p1 == P1_INITIAL_NO_ATTRIBUTES) && ctx->state == EXPORT_DATA_INITIAL) {
        ctx->coinType = U4BE(dataBuffer, 0);
        if (ctx->coinType != CONCORDIUM_COIN_TYPE_MAINNET && ctx->coinType != CONCORDIUM_COIN_TYPE_TESTNET) {
            THROW(ERROR_INVALID_PARAM);
        }
        dataBuffer += 4;
        ctx->identityProvider = U4BE(dataBuffer, 0);
        dataBuffer += 4;
        ctx->identity = U4BE(dataBuffer, 0);

        // Add non-attribute data to output
        if (p1 != P1_INITIAL_ONLY_ATTRIBUTES) {
            getIdentityData(ID_CRED_SEC, ctx->output, 32);
            getIdentityData(PRF_KEY, ctx->output + 32, 32);
            getIdentityData(BLINDING_RANDOMNESS, ctx->output + 64, 32);
            ctx->outputLength = 96;
        } else {
            ctx->outputLength = 0;
        }

        // display chosen identity index
        bin2dec(ctx->identityDisplay, sizeof(ctx->identityDisplay), ctx->identity);

        // Read attribute randomness-specific inputs
        if (p1 != P1_INITIAL_NO_ATTRIBUTES) {
            dataBuffer += 4;
            ctx->credCounter = U4BE(dataBuffer, 0);
            dataBuffer += 4;
            ctx->attributeCount = U2BE(dataBuffer, 0);
            dataBuffer += 2;
            ctx->state = EXPORT_DATA_ATTRIBUTES;

            // Ensure that our
            if (18 + ctx->attributeCount != dataLength) {
                THROW(ERROR_INVALID_PARAM);
            }

            uint8_t attributesRead = 0;
            while(attributesRead < ctx->attributeCount) {
                ctx->attributes[attributesRead] = dataBuffer[0];
                attributesRead++;
                dataBuffer++;
            }
            ctx->attributeIndex = 0;
        }
        switch (p1) {
        case P1_INITIAL_ALL:
            ux_flow_init(0, ux_export_data_all, NULL);
            break;
        case P1_INITIAL_NO_ATTRIBUTES:
            ux_flow_init(0, ux_export_data_no_attribute, NULL);
            break;
        case P1_INITIAL_ONLY_ATTRIBUTES:
            ux_flow_init(0, ux_export_data_only_attribute, NULL);
            break;
        default:
            THROW(ERROR_INVALID_PARAM);
        }
        *flags |= IO_ASYNCH_REPLY;
    } else if (p1 == P1_ATTRIBUTES && ctx->state == EXPORT_DATA_ATTRIBUTES) {
        uint8_t attributesInBatch = MIN(7, ctx->attributeCount);
        ctx->outputLength = 0;
        uint8_t i;
        for (i = 0; i < attributesInBatch; i++) {
            uint8_t attribute = ctx->attributes[ctx->attributeIndex];
            getAttributeRandomness(attribute, ctx->output + ctx->outputLength, 256 - ctx->outputLength);
            ctx->attributeIndex++;
            ctx->outputLength += 32;
        }
        ctx->attributeCount -= attributesInBatch;

        if (ctx->attributeCount < 0) {
            THROW(ERROR_INVALID_PARAM);
        }
        sendDataResponse(ctx->attributeCount == 0);
    } else {
        THROW(ERROR_INVALID_STATE);
    }
}
