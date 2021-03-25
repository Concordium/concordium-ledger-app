#include "os.h"
#include "cx.h"
#include "globals.h"
#include <stdbool.h>

// Converts bytes into uint64_t (big endian)
#define U8BE(buf, off) (((uint64_t)(U4BE(buf, off))     << 32) | ((uint64_t)(U4BE(buf, off + 4)) & 0xFFFFFFFF))

void sendUserRejection();

void sendSuccess(uint8_t tx);

void sendSuccessNoIdle();

int bin2dec(uint8_t *dst, uint64_t n);

void sendResponse(uint8_t tx, bool approve);

void getPrivateKey(uint32_t *keyPath, uint8_t keyPathLength, cx_ecfp_private_key_t *privateKey);

void getPublicKey(uint8_t *publicKeyArray);

void toHex(uint8_t *byteArray, const uint64_t len, char *asHex);

int parseKeyDerivationPath(uint8_t *dataBuffer);

void signTransactionHash(uint8_t *transactionHash, uint8_t *signedHash);

/**
 * Adds the account transaction header and transaction kind to the current
 * transaction hash.
 * @param cdata the incoming command data pointing to the start of the account transaction header
 * @param validTransactionKind the expected transaction kind, used to validate that the transaction is valid
 * @return the count of hashed bytes from cdata
 */
int hashAccountTransactionHeaderAndKind(uint8_t *cdata, uint8_t validTransactionKind);

/**
 * Adds the update header and update type to the current transaction hash.
 * @param cdata the incoming command data pointing to the start of the update header
 * @param validUpdateType the expected update type used to validate that the transaction is valid
 * @return the count of hashed bytes from cdata
 */
int hashUpdateHeaderAndType(uint8_t *cdata, uint8_t validUpdateType);
