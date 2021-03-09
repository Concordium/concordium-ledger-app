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

static tx_state_t *tx_state = &global_tx_state;

UX_STEP_NOCB(
    ux_sign_remove_baker_0_step,
    nn,
    {
      "Review",
      "transaction"
    });
UX_STEP_CB(
    ux_sign_remove_baker_1_step,
    nn,
    sendSuccessNoIdle(),
    {
      "Remove baker",
      "from pool"
    });
UX_STEP_CB(
    ux_sign_remove_baker_2_step,
    pnn,
    buildAndSignTransactionHash(),
    {
      &C_icon_validate_14,
      "Sign",
      "transaction"
    });
UX_STEP_CB(
    ux_sign_remove_baker_3_step,
    pnn,
    declineToSignTransaction(),
    {
      &C_icon_crossmark,
      "Decline to",
      "sign transaction"
    });
UX_FLOW(ux_sign_remove_baker,
    &ux_sign_remove_baker_0_step,
    &ux_sign_remove_baker_1_step,
    &ux_sign_remove_baker_2_step,
    &ux_sign_remove_baker_3_step
);

void handleSignRemoveBaker(uint8_t *dataBuffer, volatile unsigned int *flags) {
    int bytesRead = parseKeyDerivationPath(dataBuffer);
    dataBuffer += bytesRead;

    cx_sha256_init(&tx_state->hash);
    cx_hash((cx_hash_t *) &tx_state->hash, 0, dataBuffer, ACCOUNT_TRANSACTION_HEADER_LENGTH + 1, NULL, 0);
    uint8_t transactionKind = dataBuffer[ACCOUNT_TRANSACTION_HEADER_LENGTH];
    if (transactionKind != REMOVE_BAKER) {
        THROW(SW_INVALID_TRANSACTION);
    }

    ux_flow_init(0, ux_sign_remove_baker, NULL);
    *flags |= IO_ASYNCH_REPLY;
}
