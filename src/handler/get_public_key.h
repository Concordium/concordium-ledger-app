#pragma once

#include <stddef.h>   // size_t
#include <stdbool.h>  // bool
#include <stdint.h>   // uint*_t

#include "buffer.h"

#include "../types.h"

/**
 * Handler for GET_PUBLIC_KEY command. If successfully parse BIP32 path,
 * derive public key/chain code and send APDU response.
 *
 * @see G_context.bip32_path, G_context.pk_info.raw_public_key and
 *      G_context.pk_info.chain_code.
 *
 * @param[in,out] cdata
 *   Command data with BIP32 path.
 * @param[in]     display
 *   Whether to display address on screen or not.
 * @param[in]     sign_public_key
 *   Whether to sign the public key or not.
 *
 * @return zero or positive integer if success, negative integer otherwise.
 *
 */
int handler_get_public_key(buffer_t *cdata, bool display, bool sign_public_key);

/**
 * Derives the Ed25519 public key for a given BIP32 path.
 *
 * This function performs the following steps:
 * 1. Derives the private key from the provided BIP32 path
 * 2. Generates the corresponding Ed25519 public key
 * 3. Formats the public key according to the expected format (32 bytes)
 *
 * @param[in]  path              Pointer to the BIP32 path array
 * @param[in]  path_len          Length of the BIP32 path
 * @param[out] public_key_array  Buffer to store the formatted public key (must be at least 32
 * bytes)
 *
 * @return 0 on success
 * @return -1 if the derivation path is invalid
 * @return -2 if key initialization fails
 * @return -3 if public key generation fails
 *
 * @note The function securely clears sensitive private key data after use
 */
int get_public_key(uint32_t *path, size_t path_len, uint8_t *public_key_array);
