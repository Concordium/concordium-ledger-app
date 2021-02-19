#include "os.h"
#include "cx.h"
#include "globals.h"
#include <stdbool.h>

// Converts bytes into uint64_t (big endian)
#define U8BE(buf, off) (((uint64_t)(U4BE(buf, off))     << 32) | ((uint64_t)(U4BE(buf, off + 4)) & 0xFFFFFFFF))

void sendUserRejection();

void sendSuccess(uint8_t tx);

void sendSuccessNoIdle(uint8_t tx);

int bin2dec(uint8_t *dst, uint64_t n);

void sendResponse(uint8_t tx, bool approve);

void getPrivateKey(uint32_t *keyPath, uint8_t keyPathLength, cx_ecfp_private_key_t *privateKey);

void getPublicKey(uint8_t *publicKeyArray);

void toHex(uint8_t *byteArray, const uint64_t len, char *asHex);

int parseKeyDerivationPath(uint8_t *dataBuffer);

void signTransactionHash(uint8_t *transactionHash, uint8_t *signedHash);