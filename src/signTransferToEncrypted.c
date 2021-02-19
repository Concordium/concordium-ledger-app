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

static signTransferToEncrypted_t *ctx = &global.signTransferToEncrypted;
static tx_state_t *tx_state = &global_tx_state;

UX_STEP_NOCB(
    ux_sign_tranfer_to_encrypted_0_step,
    nn,
    {
      "Review",
      "transaction"
    });
UX_STEP_NOCB(
    ux_sign_tranfer_to_encrypted_1_step,
    bn_paging,
    {
      .title = "Amount to enc.",
      .text = (char *) global.signTransferToEncrypted.amount
    });
UX_STEP_CB(
    ux_sign_tranfer_to_encrypted_2_step,
    pnn,
    buildAndSignTransactionHash(),
    {
      &C_icon_validate_14,
      "Sign",
      "transaction"
    });
UX_STEP_CB(
    ux_sign_tranfer_to_encrypted_3_step,
    pnn,
    declineToSignTransaction(),
    {
      &C_icon_crossmark,
      "Decline to",
      "sign transaction"
    });
UX_FLOW(ux_sign_tranfer_to_encrypted,
    &ux_sign_tranfer_to_encrypted_0_step,
    &ux_sign_tranfer_to_encrypted_1_step,
    &ux_sign_tranfer_to_encrypted_2_step,
    &ux_sign_tranfer_to_encrypted_3_step
);

void handleSignTransferToEncrypted(uint8_t *dataBuffer, volatile unsigned int *flags) {
    int bytesRead = parseKeyDerivationPath(dataBuffer);
    dataBuffer += bytesRead;
    
    // Initialize hashing and add transaction header and transaction kind to the hash.
    cx_sha256_init(&tx_state->hash);
    cx_hash((cx_hash_t *) &tx_state->hash, 0, dataBuffer, 61, NULL, 0);
    dataBuffer += 61;

    // Parse transaction amount so it can be displayed.
    uint64_t amountToEncrypted = U8BE(dataBuffer, 0);
    bin2dec(ctx->amount, amountToEncrypted);
    cx_hash((cx_hash_t *) &tx_state->hash, 0, dataBuffer, 8, NULL, 0);
    dataBuffer += 8;

    ux_flow_init(0, ux_sign_tranfer_to_encrypted, NULL);
    *flags |= IO_ASYNCH_REPLY;
}
