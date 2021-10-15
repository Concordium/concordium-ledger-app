#include "os.h"
#include "cx.h"
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "util.h"
#include "menu.h"
#include "base58check.h"
#include "responseCodes.h"
#include "numberHelpers.h"

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
    accountSender->sender[55] = '\0';

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

int handleHeaderAndToAddress(uint8_t *cdata, uint8_t kind, uint8_t *recipientDst, size_t recipientSize) {
    // Parse the key derivation path, which should always be the first thing received
    // in a command to the Ledger application.
    int keyPathLength = parseKeyDerivationPath(cdata);
    cdata += keyPathLength;

    // Initialize the hash that will be the hash of the whole transaction, which is what will be signed
    // if the user approves.
    cx_sha256_init(&tx_state->hash);
    int headerLength = hashAccountTransactionHeaderAndKind(cdata, kind);
    cdata += headerLength;

    // Extract the recipient address and add to the hash.
    uint8_t toAddress[32];
    memmove(toAddress, cdata, 32);
    cx_hash((cx_hash_t *) &tx_state->hash, 0, toAddress, 32, NULL, 0);

    // The recipient address is in a base58 format, so we need to encode it to be
    // able to display in a human-readable way.
    if (base58check_encode(toAddress, sizeof(toAddress), recipientDst, &recipientSize) != 0) {
        // The received address bytes are not a valid base58 encoding.
        THROW(ERROR_INVALID_TRANSACTION);
    }
    recipientDst[55] = '\0';
    return keyPathLength + headerLength + 32;
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

    // Wrap in try/finally to ensure private key information is cleaned up, even if the system call fails.
    BEGIN_TRY {
        TRY {
            getPrivateKey(keyPath->keyDerivationPath, keyPath->pathLength, &privateKey);
            // Invoke the device method for generating a public-key pair.
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
