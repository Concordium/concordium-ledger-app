#include "util.h"

#include "../types.h"
#include "../constants.h"
#include <os.h>
#include <cx.h>
#include <base58.h>
#include "../globals.h"

#define l_CONST        48  // ceil((3 * ceil(log2(r))) / 16)
#define BLS_KEY_LENGTH 32
#define SEED_LENGTH    32

static const uint8_t l_bytes[2] = {0, l_CONST};

int address_to_base58(const uint8_t *address,
                      size_t address_len,
                      char *encoded_address,
                      size_t encoded_address_len) {
    if (address_len != CONCORDIUM_ADDRESS_LEN) {
        return -1;
    }
    // 1 byte for the version + 32 bytes for the address + 4 bytes for the checksum
    size_t buffer_len = 1 + CONCORDIUM_ADDRESS_LEN + 4;
    uint8_t buffer[1 + CONCORDIUM_ADDRESS_LEN + 4];
    buffer[0] = CONCORDIUM_VERSION_BYTE;
    memcpy(buffer + 1, address, CONCORDIUM_ADDRESS_LEN);

    // Calculate SHA256(SHA256(version + in)), and append the first 4 bytes to the (version + in)
    // bytes.
    uint8_t hash[32];
    cx_hash_sha256(buffer, 1 + CONCORDIUM_ADDRESS_LEN, hash, sizeof(hash));
    cx_hash_sha256(hash, sizeof(hash), hash, sizeof(hash));
    memcpy(buffer + 1 + CONCORDIUM_ADDRESS_LEN, hash, 4);

    // This function will return the number of bytes encoded, or -1 on error.
    int rtn = base58_encode(buffer, buffer_len, encoded_address, encoded_address_len);
    // Null terminate the string
    encoded_address[encoded_address_len - 1] = '\0';
    return rtn;
}

int get_private_key_from_path(uint32_t *path, size_t path_len, cx_ecfp_private_key_t *private_key) {
    int rtn = 0;
    uint8_t private_key_data[64];
    if (os_derive_bip32_with_seed_no_throw(HDW_ED25519_SLIP10,
                                           CX_CURVE_Ed25519,
                                           path,
                                           path_len,
                                           private_key_data,
                                           NULL,
                                           (unsigned char *) "ed25519 seed",
                                           12) != CX_OK) {
        rtn = -1;
    }
    if (rtn == 0 &&
        cx_ecfp_init_private_key_no_throw(CX_CURVE_Ed25519, private_key_data, 32, private_key) !=
            CX_OK) {
        rtn = -2;
    }
    explicit_bzero(private_key_data, sizeof(private_key_data));
    return rtn;
}

int bls_key_gen_from_seed(uint8_t *seed,
                          size_t seed_len,
                          uint8_t *private_key,
                          size_t private_key_len) {
    if (private_key_len != BLS_KEY_LENGTH) {
        return -1;
    }
    if (seed_len != SEED_LENGTH) {
        return -1;
    }

    uint8_t sk[l_CONST];
    uint8_t prk[32];
    uint8_t salt[32] = {
        66, 76, 83, 45, 83, 73, 71, 45, 75, 69,
        89, 71, 69, 78, 45, 83, 65, 76, 84, 45};  // Initially set to the byte representation of
                                                  // "BLS-SIG-KEYGEN-SALT-"
    size_t saltSize = 20;                         // 20 = size of initial salt seed
    uint8_t ikm[SEED_LENGTH + 1];

    memcpy(ikm, seed, SEED_LENGTH);
    ikm[SEED_LENGTH] = 0;

    do {
        cx_hash_sha256(salt, saltSize, salt, sizeof(salt));
        saltSize = sizeof(salt);
        cx_hkdf_extract(CX_SHA256, ikm, sizeof(ikm), salt, sizeof(salt), prk);
        cx_hkdf_expand(CX_SHA256,
                       prk,
                       sizeof(prk),
                       (unsigned char *) l_bytes,
                       sizeof(l_bytes),
                       sk,
                       sizeof(sk));

        if (cx_math_modm_no_throw(sk, sizeof(sk), r, sizeof(r)) != CX_OK) {
            return -1;
        }
    } while (cx_math_is_zero(sk, sizeof(sk)));

    // Skip the first 16 bytes, because they are 0 due to calculating modulo r, which is 32 bytes
    // (and sk has 48 bytes).
    memmove(private_key, sk + l_CONST - BLS_KEY_LENGTH, BLS_KEY_LENGTH);
    return 0;
}

int get_bls_private_key(uint32_t *path,
                        size_t path_len,
                        uint8_t *private_key,
                        size_t private_key_len) {
    cx_ecfp_private_key_t private_key_seed;

    int rtn = get_private_key_from_path(path, path_len, &private_key_seed);
    switch (rtn) {
        case -1:  // derivation path error
            return -1;
        case -2:  // key initialization error
            return -2;
        default:
            break;
    }
    rtn = bls_key_gen_from_seed(private_key_seed.d,
                                sizeof(private_key_seed.d),
                                private_key,
                                private_key_len);
    switch (rtn) {
        case -1:  // BLS key generation error
            return -3;
        default:
            break;
    }

    explicit_bzero(&private_key_seed, sizeof(private_key_seed));
    return 0;
}

int derivation_path_type(uint32_t *bip32_path, size_t bip32_path_len) {
    int path_type = 0;
    // Check if it is a legacy key
    if (bip32_path_len >= 5 && bip32_path[0] == LEGACY_PURPOSE) {
        path_type = 1;
        // Check if it is a governance key
        if (bip32_path_len == 5) {
            path_type += 10;
            uint32_t subtree = bip32_path[LEGACY_PATH_SUBTREE_INDEX];
            if (subtree != 1 && subtree != LEGACY_GOVERNANCE_SUBTREE) {
                path_type = -1;
            }
        }
    }
    // Check if it is a new key
    else if (bip32_path_len >= 4 && bip32_path[0] == NEW_PURPOSE) {
        path_type = 2;
    }

    return path_type;
}

void harden_derivation_path(uint32_t *bip32_path, size_t bip32_path_len) {
    for (size_t i = 0; i < bip32_path_len; i++) {
        bip32_path[i] |= HARDENED_OFFSET;
    }
}

int sign(uint8_t *m_hash, size_t m_hash_len, uint8_t *signature, size_t signature_len) {
    size_t sig_len = signature_len;
    cx_ecfp_private_key_t private_key;

    cx_err_t error = CX_OK;

    // harden the path
    harden_derivation_path(G_context.bip32_path, G_context.bip32_path_len);
    // get private key from path
    if (get_private_key_from_path(G_context.bip32_path, G_context.bip32_path_len, &private_key) !=
        0) {
        return -1;
    }
    // sign the message
    error = cx_eddsa_sign_no_throw(&private_key, CX_SHA512, m_hash, m_hash_len, signature, sig_len);

    if (error != CX_OK) {
        return -2;
    }

    PRINTF("Signature: %.*H\n", sig_len, signature);
    return 0;
}
