#include "globals.h"

static signTransferToPublic_t *ctx = &global.signTransferToPublic;
static tx_state_t *tx_state = &global_tx_state;

#define P1_INITIAL          0x00
#define P1_REMAINING_AMOUNT 0x01
#define P1_PROOF            0x02

void handleSignTransferToPublic(uint8_t *cdata,
                                uint8_t p1,
                                uint8_t dataLength,
                                volatile unsigned int *flags,
                                bool isInitialCall) {
    if (isInitialCall) {
        ctx->state = TX_TRANSFER_TO_PUBLIC_INITIAL;
    }
    uint8_t remainingDataLength = dataLength;
    if (p1 == P1_INITIAL && ctx->state == TX_TRANSFER_TO_PUBLIC_INITIAL) {
        size_t offset = parseKeyDerivationPath(cdata, remainingDataLength);
        if (offset > dataLength) {
            THROW(ERROR_BUFFER_OVERFLOW);  // Ensure safe access
        }
        cdata += offset;
        remainingDataLength -= offset;
        if (cx_sha256_init(&tx_state->hash) != CX_SHA256) {
            THROW(ERROR_FAILED_CX_OPERATION);
        }
        offset =
            hashAccountTransactionHeaderAndKind(cdata, remainingDataLength, TRANSFER_TO_PUBLIC);
        if (offset > dataLength) {
            THROW(ERROR_BUFFER_OVERFLOW);  // Ensure safe access
        }
        ctx->state = TX_TRANSFER_TO_PUBLIC_REMAINING_AMOUNT;
        // Ask the caller for the next command.
        sendSuccessNoIdle();
    } else if (p1 == P1_REMAINING_AMOUNT && ctx->state == TX_TRANSFER_TO_PUBLIC_REMAINING_AMOUNT) {
        // Hash remaining amount. Remaining amount is encrypted, and so we cannot display it.
        if (remainingDataLength < 192) {
            THROW(ERROR_BUFFER_OVERFLOW);
        }
        updateHash((cx_hash_t *)&tx_state->hash, cdata, 192);
        cdata += 192;
        remainingDataLength -= 192;
        PRINTF(
            "km-logs ---- [signTransferToPublic] (handleSignTransferToPublic) "
            "remainingDataLength1: "
            "%d\n",
            remainingDataLength);

        // Parse transaction amount so it can be displayed.
        if (remainingDataLength < 8) {
            THROW(ERROR_BUFFER_OVERFLOW);
        }
        uint64_t amountToPublic = U8BE(cdata, 0);
        amountToGtuDisplay(ctx->amount, sizeof(ctx->amount), amountToPublic);
        updateHash((cx_hash_t *)&tx_state->hash, cdata, 8);
        cdata += 8;
        remainingDataLength -= 8;
        PRINTF(
            "km-logs ---- [signTransferToPublic] (handleSignTransferToPublic) "
            "remainingDataLength2: "
            "%d\n",
            remainingDataLength);

        // Parse Recipient address
        if (remainingDataLength < 32) {
            THROW(ERROR_BUFFER_OVERFLOW);
        }
        size_t recipientAddressSize = sizeof(ctx->recipientAddress);
        if (base58check_encode(cdata, 32, ctx->recipientAddress, &recipientAddressSize) == -1) {
            PRINTF(
                "km-logs ---- [signTransferToPublic] (handleSignTransferToPublic) "
                "base58check_encode failed\n");
            THROW(ERROR_BUFFER_OVERFLOW);
        }
        ctx->recipientAddress[55] = '\0';
        PRINTF(
            "km-logs ---- [signTransferToPublic] (handleSignTransferToPublic) recipientAddress: "
            "%s\n",
            ctx->recipientAddress);
        updateHash((cx_hash_t *)&tx_state->hash, cdata, 32);
        cdata += 32;
        remainingDataLength -= 32;

        // Hash amount index
        if (remainingDataLength < 8) {
            THROW(ERROR_BUFFER_OVERFLOW);
        }
        updateHash((cx_hash_t *)&tx_state->hash, cdata, 8);
        cdata += 8;
        remainingDataLength -= 8;

        // Parse size of incoming proofs.
        if (remainingDataLength < 2) {
            THROW(ERROR_BUFFER_OVERFLOW);
        }
        ctx->proofSize = U2BE(cdata, 0);

        ctx->state = TX_TRANSFER_TO_PUBLIC_PROOF;
        sendSuccessNoIdle();
    } else if (p1 == P1_PROOF && ctx->state == TX_TRANSFER_TO_PUBLIC_PROOF) {
        updateHash((cx_hash_t *)&tx_state->hash, cdata, dataLength);

        if (ctx->proofSize == dataLength) {
            // We have received all proof bytes, continue to signing flow.
            uiSignTransferToPublicDisplay(flags);
        } else if (ctx->proofSize < dataLength) {
            // We received more proof bytes than expected, and so the received
            // transaction is invalid.
            THROW(ERROR_INVALID_TRANSACTION);
        } else {
            // There are additional bytes to be received, so ask the caller
            // for more data.
            ctx->proofSize -= dataLength;
            sendSuccessNoIdle();
        }
    } else {
        THROW(ERROR_INVALID_STATE);
    }
}
