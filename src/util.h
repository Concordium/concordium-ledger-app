#include "os.h"
#include "cx.h"
#include "globals.h"
#include <stdbool.h>

// Converts bytes into uint64_t (big endian)
#define U8BE(buf, off) (((uint64_t)(U4BE(buf, off))     << 32) | ((uint64_t)(U4BE(buf, off + 4)) & 0xFFFFFFFF))

void sendUserRejection();

void sendSuccess(uint8_t tx);

void sendResponse(uint8_t tx, bool approve);

void getPrivateKey(uint32_t accountNumber, cx_ecfp_private_key_t *privateKey);

void getPublicKey(uint32_t accountNumber, uint8_t *publicKeyArray);

void publicKeyToHex(uint8_t *publicKeyArray, const uint64_t len, char * publicKeyAsHex);