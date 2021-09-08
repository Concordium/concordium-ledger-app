#include "os.h"
#include "cx.h"
#include "globals.h"
#include <stdbool.h>
#include <string.h>

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
 * Writes the input amount of µGTU to the supplied destination as its value in
 * GTU with thousand separators. 
 * @param dst where to write the thousand separated representation of the µGTU
 * @param number the integer µGTU amount to convert to a GTU display version
 * @return number of bytes written to 'dst'
 */ 
int amountToGtuDisplay(uint8_t *dst, uint64_t microGtuAmount);

/**
 * Helper method that writes the input integer to a format that the device
 * can display on screen. The result is not string terminated.
 * @param dst where to write the text representation of the integer
 * @param number the integer to convert to characters
 * @return number of bytes written to 'dst', i.e. the number of characters in the integer 'number'
 */
int numberToText(uint8_t *dst, uint64_t number);

/**
 * Helper method that writes the input integer to a format that the device can 
 * display on the screen.
 * @param dst where to write the text representation of the integer
 * @param number the integer to convert to characters
 * @return number of bytes written to 'dst', i.e. the number of characters in the integer 'number' + 1 for string termination
 */
int bin2dec(uint8_t *dst, uint64_t number);

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
 * Helper method for converting a byte array into a character array, where the bytes
 * are translated into their hexadecimal representation. This is used for getting human-readable
 * representations of e.g. keys and credential ids. The output array is 'paginated' by inserting
 * a space after 16 characters, as this will make the Ledger pagination change page after 
 * 16 characters.
 * @param byteArray [in] the bytes to convert to paginated hex
 * @param len the length of 'byteArray', i.e. the number of bytes to convert to paginated hex
 * @param asHex [out] where to write the output hexadecimal characters
 */
void toPaginatedHex(uint8_t *byteArray, const uint64_t len, char *asHex);

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
 * Builds a human-readable representation of the identity/account path. A
 * pre-condotion for running this method is that 'parseKeyDerivation' has been
 * run prior to it.
 * @param dst [out] where to write the identity/account string
 */
void getIdentityAccountDisplay(uint8_t *dst);

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
