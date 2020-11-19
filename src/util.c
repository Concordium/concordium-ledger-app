#include "os.h"
#include "cx.h"
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "util.h"
#include "menu.h"

static accountSubtreePath_t *keyPath = &path;
static const uint32_t HARDENED_OFFSET = 0x80000000;

int bin2dec(uint8_t *dst, uint64_t n);

// Parses the key derivation path that is available in the first 2 bytes received for account instructions.
// This is used for building the key derivation path when deriving a private key.
void parseAccountSignatureKeyPath(uint8_t *dataBuffer) {
    uint8_t identity[1];
    os_memmove(identity, dataBuffer , 1);
    keyPath->identity = identity[0];

    uint8_t accountIndex[1];
    os_memmove(accountIndex, dataBuffer + 1, 1);
    keyPath->accountIndex = accountIndex[0];

    os_memmove(keyPath->displayAccount, "with #", 6);
    bin2dec(keyPath->displayAccount + 6, keyPath->accountIndex);
}

// Sends back the user rejection error code 0x6985, which indicates that
// the user has rejected the incoming command at some step in the process.
void sendUserRejection() {
    G_io_apdu_buffer[0] = 0x69;
    G_io_apdu_buffer[1] = 0x85;
    io_exchange(CHANNEL_APDU | IO_RETURN_AFTER_TX, 2);
    ui_idle();
}

// Writes the provided data into the APDU buffer and appends a success code 0x9000 to indicate
// that the command was executed successfully. The result data should already have been written
// to the APDU buffer before calling this method, and the caller should provide the correct tx offset.
void sendSuccess(uint8_t tx) {
    G_io_apdu_buffer[tx++] = 0x90;
    G_io_apdu_buffer[tx++] = 0x00;
    io_exchange(CHANNEL_APDU | IO_RETURN_AFTER_TX, tx);
    ui_idle();
}

// Send data back to the computer, but do not return to the idle screen.
void sendSuccessNoIdle(uint8_t tx) {
    G_io_apdu_buffer[tx++] = 0x90;
    G_io_apdu_buffer[tx++] = 0x00;
    io_exchange(CHANNEL_APDU | IO_RETURN_AFTER_TX, tx);
}

// Helper method that converts a byte array into a character array with the bytes translated
// into their hexadecimal representation. This is used to have a human readable representation of the
// different keys and addresses.
void toHex(uint8_t *byteArray, const uint64_t len, char *asHex) {
    static uint8_t const hex[] = "0123456789abcdef";
    for (uint64_t i = 0; i < len; i++) {
        asHex[2 * i + 0] = hex[(byteArray[i]>>4) & 0x0F];
        asHex[2 * i + 1] = hex[(byteArray[i]>>0) & 0x0F];
    }
    asHex[2 * len] = '\0';
}

// Helper method that writes the input integer to the binary format that the device can display.
int bin2dec(uint8_t *dst, uint64_t n) {
	if (n == 0) {
		dst[0] = '0';
		dst[1] = '\0';
		return 1;
	}

	// Determine the length
	int len = 0;
	for (uint64_t nn = n; nn != 0; nn /= 10) {
		len++;
	}

	// Build in big-endian order.
	for (int i = len-1; i >= 0; i--) {
		dst[i] = (n % 10) + '0';
		n /= 10;
	}
	dst[len] = '\0';
	return len;
}

void getPrivateKeyBasic(uint32_t *keyPath, uint8_t keyPathLength, cx_ecfp_private_key_t *privateKey) {
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

// Derives an account private key for signatures.
// Path = purpose'/coin_type'/0'/identity'/0'/account_index'
void getAccountSignaturePrivateKey(uint32_t identity, uint32_t accountIndex, cx_ecfp_private_key_t *privateKey) {
    uint32_t keyPath[] = {CONCORDIUM_PURPOSE | HARDENED_OFFSET, CONCORDIUM_COIN_TYPE | HARDENED_OFFSET, 0 | HARDENED_OFFSET, identity | HARDENED_OFFSET, accountIndex | HARDENED_OFFSET};
    getPrivateKeyBasic(keyPath, 5, privateKey);
}

// Gets the derived public-key for the given identity and account index. It is written to the provided byte array that
// must have a size of exactly 32 bytes.
void getPublicKey(uint32_t identity, uint32_t accountIndex, uint8_t *publicKeyArray) {
    cx_ecfp_private_key_t privateKey;
    cx_ecfp_public_key_t publicKey;

    getAccountSignaturePrivateKey(identity, accountIndex, &privateKey);

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

// Generic method that signs a transaction hash with the provided identity/account index private key.
void signTransactionHash(uint32_t identity, uint32_t accountIndex, uint8_t *transactionHash, uint8_t *signedHash) {
    // Sign the transaction hash with the private key for the given account index.
    cx_ecfp_private_key_t privateKey;

    BEGIN_TRY {
        TRY {
            getAccountSignaturePrivateKey(keyPath->identity, keyPath->accountIndex, &privateKey);
            cx_eddsa_sign(&privateKey, CX_RND_RFC6979 | CX_LAST, CX_SHA512, transactionHash, 32, NULL, 0, signedHash, 64, NULL);
        }
        FINALLY {
            // Clean up the private key, so that we cannot leak it.
            explicit_bzero(&privateKey, sizeof(privateKey));
        }
    }
    END_TRY;
}



