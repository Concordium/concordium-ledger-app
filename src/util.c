#include "os.h"
#include "cx.h"
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "util.h"
#include "menu.h"
#include "base58check.h"
#include "responseCodes.h"

static tx_state_t *tx_state = &global_tx_state;
static keyDerivationPath_t *keyPath = &path;
static accountSender_t *accountSender = &global_account_sender;
static const uint32_t HARDENED_OFFSET = 0x80000000;

int parseKeyDerivationPath(uint8_t *cdata) {
    keyPath->pathLength = cdata[0];

    // Concordium does not use key paths with a length greater than 8,
    // so if that was received, then throw an error.
    if (keyPath->pathLength > 8) {
        THROW(ERROR_INVALID_PATH);
    }

    // Each part of a key path is a uint32, parse through each part of the
    // derivation path. All paths are hardened, but we save a non-hardened
    // version that can be displayed if needed.
    for (int i = 0; i < keyPath->pathLength; ++i) {
        uint32_t node = U4BE(cdata, 1 + (i * 4));
        keyPath->rawKeyDerivationPath[i] = node;
        keyPath->keyDerivationPath[i] = node | HARDENED_OFFSET;
    }

    return 1 + (4 * keyPath->pathLength);
}

int lengthOfNumber(uint64_t number) {
	int len = 0;
	for (uint64_t nn = number; nn != 0; nn /= 10) {
		len++;
    }
    return len;
}

int numberToText(uint8_t *dst, uint64_t number) {
	if (number == 0) {
		dst[0] = '0';
		return 1;
	}
    int len = lengthOfNumber(number);

	// Build the number in big-endian order.
	for (int i = len - 1; i >= 0; i--) {
		dst[i] = (number % 10) + '0';
		number /= 10;
	}
	return len;
}

int bin2dec(uint8_t *dst, uint64_t number) {
    int characterLength = numberToText(dst, number);
    dst[characterLength] = '\0';
    return characterLength + 1;
}

/**
 * Write the display version of the decimal part of a GTU amount,
 * i.e. the numbers on the right-side of the decimal point.
 */
int decimalAmountToGtuDisplay(uint8_t *dst, uint64_t microGtuAmount) {
    // Fill with zeroes if the number is less than 6 digits,
    // so that input like 5304 become 005304 in their display version.
    int length = lengthOfNumber(microGtuAmount);
    int zeroFillLength = 6 - length;
    for (int i = 0; i < zeroFillLength; i++) {
        dst[i] = '0';
    }

    // Remove any non-significant zeroes from the number.
    // This avoids displaying numbers like 5300, as it will
    // instead become 53.
    for (int i = length - 1; i >= 0; i--) {
		uint64_t currentNumber = (microGtuAmount % 10);
        if (currentNumber != 0) {
            break;
        } else {
            microGtuAmount /= 10;
        }
	}

    return numberToText(dst + zeroFillLength, microGtuAmount) + zeroFillLength;
}

/**
 * Constructs a display text version of a micro GTU amount, so that it
 * can displayed as GTU, i.e. not as the micro version, as it is easier
 * to relate to in the GUI.
 */
int amountToGtuDisplay(uint8_t *dst, uint64_t microGtuAmount) {
    // A zero amount should be displayed as a plain '0'.
    if (microGtuAmount == 0) {
        dst[0] = '0';
        dst[1] = '\0';
        return 2;
    }

    int length = lengthOfNumber(microGtuAmount);

    // If the amount is less than than the resolution (micro), then the 
    // GTU amount has to be prefixed by '0.' as it will purely consist 
    // of the decimals.
    if (microGtuAmount < 1000000) {
        dst[0] = '0';
        dst[1] = '.';
        int length = decimalAmountToGtuDisplay(dst + 2, microGtuAmount) + 2;
        dst[length] = '\0';
        return length + 1;
    }

    // If we reach this case, then the number is greater than 1.000.000 and we will
    // need to consider thousand separators for the whole number part.
    int wholeNumberLength = length - 6;
    int current = 0;
    int separatorCount = wholeNumberLength / 3;
    if (wholeNumberLength % 3 == 0) {
        separatorCount -= 1;
    }

    // 100,000
    
    // The first 6 digits should be without thousand separators,
    // as they are part of the decimal part of the number. Write those
    // characters first to the destination output and separate with ','
    uint8_t decimalSeparatorCount = 0;
    int decimalPartLength = 0;
    uint64_t decimalPart = microGtuAmount % 1000000;
    if (decimalPart != 0) {
        decimalPartLength = decimalAmountToGtuDisplay(dst + wholeNumberLength + separatorCount + 1, decimalPart);
        dst[wholeNumberLength + separatorCount] = '.';
        decimalSeparatorCount = 1;

        // Adjust length, as we might not have exactly 6 decimals anymore, as we have removed
        // non-significant zeroes at the end of the number.
        length -= 6 - decimalPartLength;
    } else {
        // The number does not have any decimals (they are all 0), so we reduce the total
        // length of the number to remove the decimals, as we don't need them to display the number.
        length -= 6;
    }
    microGtuAmount /= 1000000;

    // Write the whole number part of the amount to the output destination. This
    // part has to have thousand separators added.
    for (int i = wholeNumberLength - 1 + separatorCount; i >= 0; i--) {
		dst[i] = (microGtuAmount % 10) + '0';
		microGtuAmount /= 10;
        
        current += 1;
        if (current == 3 && i != 0) {
            dst[i - 1] = ',';
            i--;
            current = 0;
        }
    }

    dst[length + separatorCount + decimalSeparatorCount] = '\0';
    return length + separatorCount + decimalSeparatorCount + 1;
}

// Builds a display version of the identity/account path. A pre-condition
// for running this method is that 'parseKeyDerivation' has been
// run prior to it.
void getIdentityAccountDisplay(uint8_t *dst) {
    uint32_t identityIndex = keyPath->rawKeyDerivationPath[4];
    uint32_t accountIndex = keyPath->rawKeyDerivationPath[6];

    int offset = bin2dec(dst, identityIndex) - 1;
    memmove(dst + offset, "/", 1);
    offset = offset + 1;
    offset = offset + bin2dec(dst + offset, accountIndex);
}

/**
 * Generic method for hashing and validating header and type for a transaction.
 * Use hashAccountTransactionHeaderAndKind or hashUpdateHeaderAndType 
 * instead of using this method directly.
 */ 
int hashHeaderAndType(uint8_t *cdata, uint8_t headerLength, uint8_t validType) {
    cx_hash((cx_hash_t *) &tx_state->hash, 0, cdata, headerLength, NULL, 0);
    cdata += headerLength;

    uint8_t type = cdata[0];
    if (type != validType) {
        THROW(ERROR_INVALID_TRANSACTION);
    }
    cx_hash((cx_hash_t *) &tx_state->hash, 0, cdata, 1, NULL, 0);
    cdata += 1;

    return headerLength + 1;
}

/**
 * Adds the account transaction header and the transaction kind to the hash. The 
 * transaction kind is verified to have the supplied value to prevent processing
 * invalid transactions.
 * 
 * A side effect of this method is that the sender address from the transaction header
 * is parsed and saved in a global variable, so that it is available to be displayed
 * for all account transactions.
 */
int hashAccountTransactionHeaderAndKind(uint8_t *cdata, uint8_t validTransactionKind) {
    // Parse the account sender address from the transaction header, so it can be shown.
    size_t outputSize = sizeof(accountSender->sender);
    if (base58check_encode(cdata, 32, accountSender->sender, &outputSize) != 0) {
        // The received address bytes are not a valid base58 encoding.
        THROW(ERROR_INVALID_TRANSACTION);  
    }
    accountSender->sender[50] = '\0';

    return hashHeaderAndType(cdata, ACCOUNT_TRANSACTION_HEADER_LENGTH, validTransactionKind);
}

/**
 * Adds the update header and the update type to the hash. The update
 * type is verified to have the supplied value to prevent processing
 * invalid transactions.
 */
int hashUpdateHeaderAndType(uint8_t *cdata, uint8_t validUpdateType) {
    return hashHeaderAndType(cdata, UPDATE_HEADER_LENGTH, validUpdateType);
}

void sendUserRejection() {
    G_io_apdu_buffer[0] = ERROR_REJECTED_BY_USER >> 8;
    G_io_apdu_buffer[1] = ERROR_REJECTED_BY_USER & 0xFF;
    io_exchange(CHANNEL_APDU | IO_RETURN_AFTER_TX, 2);
    ui_idle();
}

void sendSuccess(uint8_t tx) {
    G_io_apdu_buffer[tx++] = SUCCESS >> 8;
    G_io_apdu_buffer[tx++] = SUCCESS & 0xFF;
    io_exchange(CHANNEL_APDU | IO_RETURN_AFTER_TX, tx);
    ui_idle();
}

void sendSuccessNoIdle() {
    sendSuccessResultNoIdle(0);
}

void sendSuccessResultNoIdle(uint8_t tx) {
    G_io_apdu_buffer[tx++] = SUCCESS >> 8;
    G_io_apdu_buffer[tx++] = SUCCESS & 0xFF;
    io_exchange(CHANNEL_APDU | IO_RETURN_AFTER_TX, tx);
}

void toHex(uint8_t *byteArray, const uint64_t len, char *asHex) {
    static uint8_t const hex[] = "0123456789abcdef";
    for (uint64_t i = 0; i < len; i++) {
        asHex[2 * i + 0] = hex[(byteArray[i]>>4) & 0x0F];
        asHex[2 * i + 1] = hex[(byteArray[i]>>0) & 0x0F];
    }
    asHex[2 * len] = '\0';
}

void getPrivateKey(uint32_t *keyPath, uint8_t keyPathLength, cx_ecfp_private_key_t *privateKey) {
    uint8_t privateKeyData[32];

    // Invoke the device methods for generating a private key.
    // Wrap in try/finally to ensure that private key information is cleaned up, even if a system call fails.
    BEGIN_TRY {
        TRY {
            os_perso_derive_node_bip32_seed_key(HDW_ED25519_SLIP10, CX_CURVE_Ed25519, keyPath, keyPathLength, privateKeyData, NULL, (unsigned char*) "ed25519 seed", 12);
            cx_ecfp_init_private_key(CX_CURVE_Ed25519, privateKeyData, 32, privateKey);
        }
        FINALLY {
            // Clean up the private key seed data, so that we cannot leak it.
            explicit_bzero(&privateKeyData, sizeof(privateKeyData));
        }
    }
    END_TRY;
}

void getPublicKey(uint8_t *publicKeyArray) {
    cx_ecfp_private_key_t privateKey;
    cx_ecfp_public_key_t publicKey;

    getPrivateKey(keyPath->keyDerivationPath, keyPath->pathLength, &privateKey);

    // Invoke the device method for generating a public-key pair.
    // Wrap in try/finally to ensure private key information is cleaned up, even if the system call fails.
    BEGIN_TRY {
        TRY {
            cx_ecfp_generate_pair(CX_CURVE_Ed25519, &publicKey, &privateKey, 1);
        }
        FINALLY {
            // Clean up the private key as we are done using it, so that we cannot leak it.
            explicit_bzero(&privateKey, sizeof(privateKey));
        }
    }
    END_TRY;

    // Build the public-key bytes in the expected format.
    for (int i = 0; i < 32; i++) {
        publicKeyArray[i] = publicKey.W[64 - i];
    }
    if ((publicKey.W[32] & 1) != 0) {
        publicKeyArray[31] |= 0x80;
    }
}

// Generic method that signs the input with the key given by the derivation path that
// has been loaded into keyPath.
void sign(uint8_t *input, uint8_t *signatureOnInput) {
    cx_ecfp_private_key_t privateKey;

    BEGIN_TRY {
        TRY {
            getPrivateKey(keyPath->keyDerivationPath, keyPath->pathLength, &privateKey);
            cx_eddsa_sign(&privateKey, CX_RND_RFC6979 | CX_LAST, CX_SHA512, input, 32, NULL, 0, signatureOnInput, 64, NULL);
        }
        FINALLY {
            // Clean up the private key, so that we cannot leak it.
            explicit_bzero(&privateKey, sizeof(privateKey));
        }
    }
    END_TRY;
}
