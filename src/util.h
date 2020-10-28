#include "os.h"
#include "cx.h"
#include "globals.h"
#include <stdbool.h>

void sendUserRejection();

void sendSuccess(uint8_t tx);

void sendResponse(uint8_t tx, bool approve);

void getPrivateKey(uint32_t accountNumber, cx_ecfp_private_key_t *privateKey);

void getPublicKey(uint32_t accountNumber, uint8_t *publicKeyArray);

void publicKeyToHex(uint8_t *publicKeyArray, const uint64_t len, char * publicKeyAsHex);