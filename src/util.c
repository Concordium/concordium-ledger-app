#include "os.h"
#include "cx.h"
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "util.h"
#include "menu.h"

static tx_state_t *tx_state = &global_tx_state;
static keyDerivationPath_t *keyPath = &path;
static const uint32_t HARDENED_OFFSET = 0x80000000;

int parseKeyDerivationPath(uint8_t *cdata) {
    keyPath->pathLength = cdata[0];

    // Concordium does not use key paths with a length greater than 8,
    // so if that was received, then throw an error.
    if (keyPath->pathLength > 8) {
        THROW(SW_INVALID_PATH);
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

int bin2dec(uint8_t *dst, uint64_t number) {
	if (number == 0) {
		dst[0] = '0';
		dst[1] = '\0';
		return 1;
	}

	// Determine the length of the text representation.
	int len = 0;
	for (uint64_t nn = number; nn != 0; nn /= 10) {
		len++;
	}

	// Build the number in big-endian order.
	for (int i = len - 1; i >= 0; i--) {
		dst[i] = (number % 10) + '0';
		number /= 10;
	}
	dst[len] = '\0';
	return len;
}

// Builds a display version of the identity/account path. A pre-condition
// for running this method is that 'parseKeyDerivation' has been
// run prior to it.
void getIdentityAccountDisplay(uint8_t *dst) {
    uint32_t identityIndex = keyPath->rawKeyDerivationPath[4];
    uint32_t accountIndex = keyPath->rawKeyDerivationPath[6];

    int offset = bin2dec(dst, identityIndex);
    os_memmove(dst + offset, "/", 1);
    offset = offset + 1;
    offset = offset + bin2dec(dst + offset, accountIndex);
    os_memmove(dst + offset, "\0", 1);
}

/**
 * Generic method for hashing and validating header and type for a transaction.
 * Use hashAccountTransactionHeaderAndKind or hashAccountTransactionHeaderAndKind 
 * instead of using this method directly.
 */ 
int hashHeaderAndType(uint8_t *cdata, uint8_t headerLength, uint8_t validType) {
    cx_hash((cx_hash_t *) &tx_state->hash, 0, cdata, headerLength, NULL, 0);
    cdata += headerLength;

    uint8_t type = cdata[0];
    if (type != validType) {
        THROW(SW_INVALID_TRANSACTION);
    }
    cx_hash((cx_hash_t *) &tx_state->hash, 0, cdata, 1, NULL, 0);
    cdata += 1;

    return headerLength + 1;
}

/**
 * Adds the account transaction header and the transaction kind to the hash. The 
 * transaction kind is verified to have the supplied value to prevent processing
 * invalid transactions.
 */
int hashAccountTransactionHeaderAndKind(uint8_t *cdata, uint8_t validTransactionKind) {
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
    G_io_apdu_buffer[0] = 0x69;
    G_io_apdu_buffer[1] = 0x85;
    io_exchange(CHANNEL_APDU | IO_RETURN_AFTER_TX, 2);
    ui_idle();
}

void sendSuccess(uint8_t tx) {
    G_io_apdu_buffer[tx++] = 0x90;
    G_io_apdu_buffer[tx++] = 0x00;
    io_exchange(CHANNEL_APDU | IO_RETURN_AFTER_TX, tx);
    ui_idle();
}

void sendSuccessNoIdle() {
    G_io_apdu_buffer[0] = 0x90;
    G_io_apdu_buffer[1] = 0x00;
    io_exchange(CHANNEL_APDU | IO_RETURN_AFTER_TX, 2);
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
            os_perso_derive_node_bip32_seed_key(HDW_ED25519_SLIP10, CX_CURVE_Ed25519, keyPath, keyPathLength, privateKeyData, NULL, NULL, 0);
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
