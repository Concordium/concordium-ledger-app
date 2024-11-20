#pragma once

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include "../types.h"
#include <os.h>


/**
 * BLS12-381 subgroup G1's order:
 */
static const uint8_t r[32] = {0x73, 0xed, 0xa7, 0x53, 0x29, 0x9d, 0x7d, 0x48, 0x33, 0x39, 0xd8,
                              0x08, 0x09, 0xa1, 0xd8, 0x05, 0x53, 0xbd, 0xa4, 0x02, 0xff, 0xfe,
                              0x5b, 0xfe, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x01};

/**
 * Prepare the display for the identity index and account display.
 */
int get_identity_index_account_display();

/**
 * Get the BLS private key from the path.
 */
int get_bls_private_key(uint32_t *path, size_t path_len, uint8_t *private_key, size_t private_key_len);

/**
 * Convert an address to a base58 encoded string.
 */
int address_to_base58(const uint8_t *address, size_t address_len, char *encoded_address, size_t encoded_address_len);

/**
 * Derive a private key from a BIP32 path.
 *
 * @param[in] path          Pointer to the BIP32 path
 * @param[in] path_len      Length of the BIP32 path
 * @param[out] private_key  Pointer to the private key
 *
 * @return 0 on success, -1 on error
 */
int get_private_key_from_path(uint32_t *path, size_t path_len, cx_ecfp_private_key_t *private_key);

/**
 * Generate a BLS private key from a seed.
 *
 * @param[in] seed              Input seed bytes
 * @param[in] seed_len          Length of the seed
 * @param[out] private_key      Buffer to store the generated BLS private key
 * @param[in] private_key_len   Length of the private key buffer
 *
 * @return 0 on success, -1 on error
 */
int bls_key_gen_from_seed(uint8_t *seed, size_t seed_len, uint8_t *private_key, size_t private_key_len);