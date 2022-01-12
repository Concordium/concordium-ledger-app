#ifndef _CONCORDIUM_APP_UTIL_H_
#define _CONCORDIUM_APP_UTIL_H_

#include <stdbool.h>
#include <string.h>

#include "globals.h"
#include "numberHelpers.h"
#include "os.h"

#define MAX_MEMO_SIZE 256
#define MAX_DATA_SIZE (MAX_MEMO_SIZE)

/**
 * BLS12-381 subgroup G1's order:
 */
static const uint8_t r[32] = {0x73, 0xed, 0xa7, 0x53, 0x29, 0x9d, 0x7d, 0x48, 0x33, 0x39, 0xd8,
                              0x08, 0x09, 0xa1, 0xd8, 0x05, 0x53, 0xbd, 0xa4, 0x02, 0xff, 0xfe,
                              0x5b, 0xfe, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x01};

/**
 * Converts bytes into uint64_t (big endian).
 */
#define U8BE(buf, off) (((uint64_t)(U4BE(buf, off)) << 32) | ((uint64_t)(U4BE(buf, off + 4)) & 0xFFFFFFFF))

/**
 * Send a user rejection back to the caller, which will indicate to
 * the caller that the user has rejected the incoming command at some
 * step in the process, i.e. if the user does not want to sign the
 * incoming transaction.
 *
 * After sending the rejection the display will return to the menu.
 */
void sendUserRejection();

/**
 * Send a success back to the caller without returning the display to the
 * idle menu. This method should be used in instructions that span multiple
 * commands to avoid resetting the display back to the menu inbetween commands.
 */
void sendSuccessNoIdle();

/**
 * Send a success with a result back to the caller without returning the display to the
 * idle menu.
 */
void sendSuccessResultNoIdle(uint8_t tx);

/**
 * Sends a success back to the caller, and then returns to the menu screen. The result data
 * should already have been written to the APDU buffer before calling this method, and the caller
 * should provide the correct tx offset, i.e. the number of bytes already written to the APDU buffer.
 * @param tx number of bytes written to the APDU buffer that should be sent back to the caller
 */
void sendSuccess(uint8_t tx);

/**
 * Gets the private-key for the provided key path.
 *
 * Note that any method using this method MUST zero the private key right after use of the private key,
 * as to limit any risk of leaking a private key.
 *
 * @param keyPath the key derivation path to get the private key for
 * @param keyPathLength length of the key derivation path
 * @param privateKey [out] where to write the derived private key to
 */
void getPrivateKey(uint32_t *keyPath, uint8_t keyPathLength, cx_ecfp_private_key_t *privateKey);

/**
 * Gets the public-key for the keypath that has been loaded into the state. It is a
 * pre-condition that 'parseKeyDerivation' has been run prior to this function.
 * @param publicKeyArray [out] the public-key is written here
 */
void getPublicKey(uint8_t *publicKeyArray);

/**
 * Parses the key derivation path for the command to be executed. This method should
 * be run as the first thing on all commands that require identity/account or governance
 * paths. The method can handle dynamic length paths as account and governance key paths
 * differ in length.
 * @return number of bytes received in the key derivation path
 */
int parseKeyDerivationPath(uint8_t *cdata);

/**
 * Signs the input with the private key for the loaded key derivation path.
 * It is a pre-condition for running this method that 'parseKeyDerivation' has been
 * run prior to it.
 * @param input [in] the hash to be signed
 * @param signatureOnInput [out] the signature on 'input'
 */
void sign(uint8_t *input, uint8_t *signatureOnInput);

/**
 * Builds a human-readable representation of the identity/account path.
 * @param dst [out] where to write the identity/account string
 * @param dstLength length of dst
 * @param identityIndex
 * @param accountIndex
 */
void getIdentityAccountDisplay(uint8_t *dst, size_t dstLength, uint32_t identityIndex, uint32_t accountIndex);

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

/**
 * Adds the account transaction header and the recipient address to the transaction hash, and
 * writes the base58 encoded recipient address for later display.
 * @param cdata the incoming command data pointing to the start of the input, i.e. with the key path at the start
 * @param kind the transaction type
 * @param recipientDst the destination where to write the base58 encoded recipient address
 * @param recipientSize the size of the recipient destination
 */
int handleHeaderAndToAddress(uint8_t *cdata, uint8_t kind, uint8_t *recipientDst, size_t recipientSize);

/**
 * Calculates a BLS12-381 private-key using the seed at the provided key path.
 *
 * Note that any method using this method MUST zero the private key right after use of the private key,
 * as to limit any risk of leaking a private key.
 *
 * @param keyPath the key derivation path to get the private key seed from
 * @param keyPathLength length of the key derivation path
 * @param privateKey [out] where to write the derived private key to
 * @param privateKeyLength length of privateKey
 */
void getBlsPrivateKey(uint32_t *keyPath, uint8_t keyPathLength, uint8_t *privateKey, size_t privateKeyLength);

#endif
