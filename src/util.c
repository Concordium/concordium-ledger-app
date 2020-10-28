#include "os.h"
#include "cx.h"
#include <stdbool.h>
#include <stdlib.h>
#include "util.h"
#include "menu.h"

static const uint32_t HARDENED_OFFSET = 0x80000000;

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

// Helper method that converts a byte array into a character array with the bytes translated
// into their hexadecimal representation. This is used to have a human readable representation of the
// public-key to present to the user.
void publicKeyToHex(uint8_t *publicKeyArray, const uint64_t len, char * publicKeyAsHex) {
    static uint8_t const hex[] = "0123456789abcdef";
    for (uint64_t i = 0; i < len; i++) {
        publicKeyAsHex[2 * i + 0] = hex[(publicKeyArray[i]>>4) & 0x0F];
        publicKeyAsHex[2 * i + 1] = hex[(publicKeyArray[i]>>0) & 0x0F];
    }
    publicKeyAsHex[2 * len] = '\0';
}

// Method that gets the private key for the given accountNumber. Make sure to clean up memory right after
// using the privateKey.
void getPrivateKey(uint32_t accountNumber, cx_ecfp_private_key_t *privateKey) {
    uint8_t privateKeyData[32];

    // The Concordium specific BIP32 path.
    uint32_t bip32Path[] = {44 | HARDENED_OFFSET, CONCORDIUM_COIN_TYPE | HARDENED_OFFSET, accountNumber | HARDENED_OFFSET};

    // Invoke the device methods for generating a private key.
    os_perso_derive_node_bip32_seed_key(HDW_ED25519_SLIP10, CX_CURVE_Ed25519, bip32Path, CONCORDIUM_BIP32_PATH_LENGTH, privateKeyData, NULL, NULL, 0);
    cx_ecfp_init_private_key(CX_CURVE_Ed25519, privateKeyData, 32, privateKey);

   // Clean up the private key seed data to avoid leaking information.
    os_memset(privateKeyData, 0, sizeof(privateKeyData));
}

// Gets the derived public-key for the given account number. It is written to the provided byte array that must have
// a size of exactly 32 bytes.
void getPublicKey(uint32_t accountNumber, uint8_t *publicKeyArray) {
    cx_ecfp_private_key_t privateKey;
    cx_ecfp_public_key_t publicKey;

    getPrivateKey(accountNumber, &privateKey);

    // Invoke the device method for generating a public-key pair.
    cx_ecfp_generate_pair(CX_CURVE_Ed25519, &publicKey, &privateKey, 1);

    // Clean up the private key as we are done using it, so that we do not leak it.
    os_memset(&privateKey, 0, sizeof(privateKey));

    // Build the public-key bytes in the expected format.
    for (int i = 0; i < 32; i++) {
        publicKeyArray[i] = publicKey.W[64 - i];
    }
    if ((publicKey.W[32] & 1) != 0) {
        publicKeyArray[31] |= 0x80;
    }
}