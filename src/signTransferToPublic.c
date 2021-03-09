#include <os.h>
#include <os_io_seproxyhal.h>
#include "cx.h"
#include <stdint.h>
#include "menu.h"
#include "util.h"
#include <string.h>
#include "base58check.h"
#include <stdio.h>
#include "sign.h"

static signTransferToPublic_t *ctx = &global.signTransferToPublic;
static tx_state_t *tx_state = &global_tx_state;

UX_STEP_NOCB(
    ux_sign_transfer_to_public_0_step,
    nn,
    {
      "Review",
      "transaction"
    });
UX_STEP_CB(
    ux_sign_transfer_to_public_1_step,
    bn_paging,
    sendSuccessNoIdle(),
    {
      .title = "Amount to public",
      .text = (char *) global.signTransferToPublic.amount
    });
UX_FLOW(ux_sign_transfer_to_public,
    &ux_sign_transfer_to_public_0_step,
    &ux_sign_transfer_to_public_1_step
);

#define P1_INITIAL          0x00
#define P1_REMAINING_AMOUNT 0x01
#define P1_PROOF            0x02

void handleSignTransferToPublic(uint8_t *dataBuffer, uint8_t p1, uint8_t dataLength, volatile unsigned int *flags) {
    if (p1 == P1_INITIAL) {
        int bytesRead = parseKeyDerivationPath(dataBuffer);
        dataBuffer += bytesRead;
        
        // Initialize hashing and add transaction header and transaction kind to the hash.
        cx_sha256_init(&tx_state->hash);
        cx_hash((cx_hash_t *) &tx_state->hash, 0, dataBuffer, ACCOUNT_TRANSACTION_HEADER_LENGTH + 1, NULL, 0);
        dataBuffer += ACCOUNT_TRANSACTION_HEADER_LENGTH + 1;

        // Ask the computer to continue with the protocol
        sendSuccessNoIdle();
    } else if (p1 == P1_REMAINING_AMOUNT) {
        // Hash remaining amount
        cx_hash((cx_hash_t *) &tx_state->hash, 0, dataBuffer, 192, NULL, 0);
        dataBuffer += 192;

        // Parse transaction amount so it can be displayed.
        uint64_t amountToEncrypted = U8BE(dataBuffer, 0);
        bin2dec(ctx->amount, amountToEncrypted);
        cx_hash((cx_hash_t *) &tx_state->hash, 0, dataBuffer, 8, NULL, 0);
        dataBuffer += 8;

        // Hash amount index
        cx_hash((cx_hash_t *) &tx_state->hash, 0, dataBuffer, 8, NULL, 0);
        dataBuffer += 8;

        // Parse size of incoming proofs.
        ctx->proofSize = U2BE(dataBuffer, 0);

        ux_flow_init(0, ux_sign_transfer_to_public, NULL);
        *flags |= IO_ASYNCH_REPLY;
    } else if (p1 == P1_PROOF) {
        // FIXME: This is duplicated code, this could probably be moved to a shared class.
        cx_hash((cx_hash_t *) &tx_state->hash, 0, dataBuffer, dataLength, NULL, 0);
        ctx->proofSize -= dataLength;

        if (ctx->proofSize == 0) {
            // We have received all proof bytes, continue to signing flow.
            ux_flow_init(0, ux_sign_flow_shared, NULL);
            *flags |= IO_ASYNCH_REPLY;
        } else if (ctx->proofSize < 0) {
            // We received more proof bytes than expected.
            THROW(SW_INVALID_STATE);    
        } else {
            // There are more bytes to be received. Ask the computer for more.
            sendSuccessNoIdle();
        }
    } else {
        THROW(SW_INVALID_STATE);
    }
}
