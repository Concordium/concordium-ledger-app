#include "util.h"

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "base58check.h"
#include "cx.h"
#include "menu.h"
#include "numberHelpers.h"
#include "os.h"
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

/**
 * Generic method for hashing and validating header and type for a transaction.
 * Use hashAccountTransactionHeaderAndKind or hashUpdateHeaderAndType
 * instead of using this method directly.
 */
int hashHeaderAndType(uint8_t *cdata, uint8_t headerLength, uint8_t validType) {
    updateHash((cx_hash_t *) &tx_state->hash, cdata, headerLength);
    cdata += headerLength;

    uint8_t type = cdata[0];
    if (type != validType) {
        THROW(ERROR_INVALID_TRANSACTION);
    }
    updateHash((cx_hash_t *) &tx_state->hash, cdata, 1);

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
    updateHash((cx_hash_t *) &tx_state->hash, toAddress, 32);

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

void getIdentityAccountDisplay(uint8_t *dst, size_t dstLength, uint32_t identityIndex, uint32_t accountIndex) {
    int offset = numberToText(dst, dstLength, identityIndex);
    memmove(dst + offset, "/", 1);
    offset += 1;
    bin2dec(dst + offset, dstLength - offset, accountIndex);
}

/**
 * Used to validate that an error result code from a Ledger library call
 * is equal CX_OK. If it is not CX_OK, then throw an ERROR_FAILED_CX_OPERATION
 * error that should be sent back to the callee.
 */
void ensureNoError(cx_err_t errorCode) {
    // TODO An improvement would be to stop using THROW for the control flow like this
    // and to explicitly send back the error instead and then stop the flow.
    // This implementation is a quick patch to the changes made to the Ledger SDK to
    // mimic the old library functions that would do a similar throw.

    if (errorCode != CX_OK) {
        THROW(ERROR_FAILED_CX_OPERATION);
    }
}

void getPrivateKey(uint32_t *keyPathInput, uint8_t keyPathLength, cx_ecfp_private_key_t *privateKey) {
    uint8_t privateKeyData[64];

    // Invoke the device methods for generating a private key.
    // Wrap in try/finally to ensure that private key information is cleaned up, even if a system call fails.
    BEGIN_TRY {
        TRY {
            ensureNoError(os_derive_bip32_with_seed_no_throw(
                HDW_ED25519_SLIP10,
                CX_CURVE_Ed25519,
                keyPathInput,
                keyPathLength,
                privateKeyData,
                NULL,
                (unsigned char *) "ed25519 seed",
                12));
            ensureNoError(cx_ecfp_init_private_key_no_throw(CX_CURVE_Ed25519, privateKeyData, 32, privateKey));
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
            ensureNoError(cx_ecfp_generate_pair_no_throw(CX_CURVE_Ed25519, &publicKey, &privateKey, 1));
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
            ensureNoError(cx_eddsa_sign_no_throw(&privateKey, CX_SHA512, input, 32, signatureOnInput, 64));
        }
        FINALLY {
            // Clean up the private key, so that we cannot leak it.
            explicit_bzero(&privateKey, sizeof(privateKey));
        }
    }
    END_TRY;
}

#define l_CONST        48  // ceil((3 * ceil(log2(r))) / 16)
#define BLS_KEY_LENGTH 32
#define SEED_LENGTH    32

void hash(
    cx_hash_t *hashContext,
    uint32_t mode,
    const unsigned char *in,
    unsigned int len,
    unsigned char *out,
    unsigned int out_len) {
    ensureNoError(cx_hash_no_throw(hashContext, mode, in, len, out, out_len));
}

void updateHash(cx_hash_t *hashContext, const unsigned char *in, unsigned int len) {
    return hash(hashContext, 0, in, len, NULL, 0);
}

// We must declare the functions for the static analyzer to be happy. Ideally we would have
// access to the declarations from the Ledger SDK.
void cx_hkdf_extract(
    const cx_md_t hash_id,
    const unsigned char *ikm,
    unsigned int ikm_len,
    unsigned char *salt,
    unsigned int salt_len,
    unsigned char *prk);
void cx_hkdf_expand(
    const cx_md_t hash_id,
    const unsigned char *prk,
    unsigned int prk_len,
    unsigned char *info,
    unsigned int info_len,
    unsigned char *okm,
    unsigned int okm_len);

static const uint8_t l_bytes[2] = {0, l_CONST};

/** This implements the bls key generation algorithm specified in
 * https://datatracker.ietf.org/doc/html/draft-irtf-cfrg-bls-signature-04#section-2.3, The optional parameter key_info
 * is hardcoded to an empty string. Uses sha256 as the hash function. The generated key has length 32, and dst should
 * have atleast that length, or the function throws an error.
 */
void blsKeygen(const uint8_t *seed, size_t seedLength, uint8_t *dst, size_t dstLength) {
    if (dstLength < BLS_KEY_LENGTH) {
        THROW(ERROR_BUFFER_OVERFLOW);
    } else if (seedLength != SEED_LENGTH) {
        THROW(ERROR_INVALID_TRANSACTION);
    }

    uint8_t sk[l_CONST];
    uint8_t prk[32];
    uint8_t salt[32] = {
        66, 76, 83, 45, 83, 73, 71, 45, 75, 69,
        89, 71, 69, 78, 45, 83, 65, 76, 84, 45};  // Initially set to the byte representation of "BLS-SIG-KEYGEN-SALT-"
    size_t saltSize = 20;                         // 20 = size of initial salt seed
    uint8_t ikm[SEED_LENGTH + 1];

    memcpy(ikm, seed, SEED_LENGTH);
    ikm[SEED_LENGTH] = 0;

    do {
        cx_hash_sha256(salt, saltSize, salt, sizeof(salt));
        saltSize = sizeof(salt);
        cx_hkdf_extract(CX_SHA256, ikm, sizeof(ikm), salt, sizeof(salt), prk);
        cx_hkdf_expand(CX_SHA256, prk, sizeof(prk), (unsigned char *) l_bytes, sizeof(l_bytes), sk, sizeof(sk));

        ensureNoError(cx_math_modm_no_throw(sk, sizeof(sk), r, sizeof(r)));
    } while (cx_math_is_zero(sk, sizeof(sk)));

    // Skip the first 16 bytes, because they are 0 due to calculating modulo r, which is 32 bytes (and sk has 48 bytes).
    memmove(dst, sk + l_CONST - BLS_KEY_LENGTH, BLS_KEY_LENGTH);
}

void getBlsPrivateKey(uint32_t *keyPathInput, uint8_t keyPathLength, uint8_t *privateKey, size_t privateKeySize) {
    cx_ecfp_private_key_t privateKeySeed;
    BEGIN_TRY {
        TRY {
            getPrivateKey(keyPathInput, keyPathLength, &privateKeySeed);
            blsKeygen(privateKeySeed.d, sizeof(privateKeySeed.d), privateKey, privateKeySize);
        }
        FINALLY {
            explicit_bzero(&privateKeySeed, sizeof(privateKeySeed));
        }
    }
    END_TRY;
}
