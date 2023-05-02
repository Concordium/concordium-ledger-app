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

UX_STEP_NOCB(
    ux_export_data_attribute_step,
    nn,
    {"Export private", "attribute data"});
// TODO: which identity/credential

UX_STEP_CB(ux_export_data_accept_step, pb, sendSuccessNoIdle(), {&C_icon_validate_14, "Accept"});
UX_STEP_CB(ux_export_data_decline_step, pb, sendUserRejection(), {&C_icon_crossmark, "Decline"});
UX_FLOW(ux_export_data_attribute, &ux_export_data_attribute_step, &ux_export_data_accept_step, &ux_export_data_decline_step);

void writeData(uint8_t type, uint8_t *dst, size_t dstLength) {
    uint8_t privateKey[32];
    BEGIN_TRY {
        TRY {
            ctx->path[4] = type | HARDENED_OFFSET;
            getBlsPrivateKey(ctx->path, ctx->pathLength, dst, dstLength);
        }
        FINALLY {
            explicit_bzero(&privateKey, sizeof(privateKey));
        }
    }
    END_TRY;
}

void sendData(const uint8_t *data, size_t dataLength, bool final) {
    memmove(G_io_apdu_buffer, data, dataLength);
    if (final) {
        sendSuccess(dataLength);
    } else {
        sendSuccessResultNoIdle(dataLength);
    }
}

#define ATTRIBUTE_TYPE 0x05

#define P1_INITIAL     0x00
#define P1_INITIAL_ALL     0x01
#define P1_ATTRIBUTES     0x02

// Export the BLS keys (For mainnet)
#define P2_KEY_MAINNET 0x00
// Export the BLS keys (For testnet)
#define P2_KEY_TESTNET 0x01

void handleExportPrivateIdentityData(uint8_t *dataBuffer, uint8_t p1, uint8_t p2, volatile unsigned int *flags, bool isInitialCall) {
    if (isInitialCall) {
        ctx->state = EXPORT_DATA_INITIAL;
    }

    if ((p1 == P1_INITIAL || p1 == P1_INITIAL_ALL) && ctx->state == EXPORT_DATA_INITIAL) {
        uint32_t identityProvider = U4BE(dataBuffer, 0);
        dataBuffer += 4;
        uint32_t identity = U4BE(dataBuffer, 0);
        dataBuffer += 4;
        uint32_t credCounter = U4BE(dataBuffer, 0);
        dataBuffer += 4;
        uint32_t attributeCount = U4BE(dataBuffer, 0);
        ctx->attributeCount = attributeCount;

        uint32_t coinType;
        if (p2 == P2_KEY_MAINNET) {
            coinType = CONCORDIUM_COIN_TYPE_MAINNET;
        } else if (p2 == P2_KEY_TESTNET) {
            coinType = CONCORDIUM_COIN_TYPE_TESTNET;
        } else {
            THROW(ERROR_INVALID_PARAM);
        }

        uint32_t keyDerivationPath[6] = {
            CONCORDIUM_PURPOSE | HARDENED_OFFSET,
            coinType | HARDENED_OFFSET,
            identityProvider | HARDENED_OFFSET,
            identity | HARDENED_OFFSET,
            0 | HARDENED_OFFSET, // Placeholder
            credCounter | HARDENED_OFFSET
        };
        ctx->pathLength = 7;
        memmove(ctx->path, keyDerivationPath, sizeof(keyDerivationPath));
        ctx->state = EXPORT_DATA_ATTRIBUTES;
        ux_flow_init(0, ux_export_data_attribute, NULL);
        *flags |= IO_ASYNCH_REPLY;
    } else if (p1 == P1_ATTRIBUTES && ctx->state == EXPORT_DATA_ATTRIBUTES) {
        uint8_t attributesInBatch = dataBuffer[0];
        uint8_t output[256] = {};
        uint8_t i;
        for (i = 0; i < attributesInBatch; i++) {
            uint32_t attribute = U4BE(dataBuffer, 0);
            ctx->path[6] = attribute | HARDENED_OFFSET;
            writeData(ATTRIBUTE_TYPE, output + (i * 32), 256 - (32 * i));
        }
        ctx->attributeCount -= attributesInBatch;

        if (ctx->attributeCount < 0) {
            THROW(ERROR_INVALID_PARAM);
        }
        sendData(output, 32 * i, ctx->attributeCount == 0);
    } else {
        THROW(ERROR_INVALID_STATE);
    }
}

