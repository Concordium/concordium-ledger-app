#include "os.h"

#include "common/ui/display.h"
#include "common/sign.h"
#include "common/util.h"
#include "signTransferToEncrypted.h"
#include "globals.h"

static signTransferToEncrypted_t *ctx = &global.signTransferToEncrypted;
static tx_state_t *tx_state = &global_tx_state;

void handleSignTransferToEncrypted(uint8_t *cdata, volatile unsigned int *flags) {
    cdata += parseKeyDerivationPath(cdata);
    cx_sha256_init(&tx_state->hash);
    cdata += hashAccountTransactionHeaderAndKind(cdata, TRANSFER_TO_ENCRYPTED);

    // Parse transaction amount so it can be displayed.
    uint64_t amountToEncrypted = U8BE(cdata, 0);
    amountToGtuDisplay(ctx->amount, sizeof(ctx->amount), amountToEncrypted);
    updateHash((cx_hash_t *) &tx_state->hash, cdata, 8);

    uiSignTransferToEncryptedDisplay(flags);
}
