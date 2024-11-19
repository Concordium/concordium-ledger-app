#pragma once

#include "buffer.h"
#include "cx.h"

/**
 * Handler for VERIFY_ADDRESS command. Computes the address from
 * identity index and credential counter and displays it on the screen.
 *
 * The user has to confirm the address on the screen matches the one
 * from the dApp.
 *
 * @param[in,out] cdata
 *   Command data with identity index and credential counter.
 *
 * @return zero or positive integer if success, negative integer otherwise.
 */
int handler_verify_address(buffer_t *cdata);

/**
 * Calculates the credential  ID from the given prf key and credential counter.
 * The size of the computed credential ID is 48 bytes.
 */
cx_err_t get_credential_id(uint8_t *prf_key, size_t prf_key_len, uint32_t credential_counter, uint8_t *credential_id, size_t credential_id_len);
