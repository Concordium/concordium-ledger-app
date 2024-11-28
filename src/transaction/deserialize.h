#pragma once

#include "buffer.h"

#include "types.h"
#include "../types.h"

/**
 * Deserialize raw transaction in structure.
 *
 * @param[in, out] buf
 *   Pointer to buffer with serialized transaction.
 * @param[out]     tx
 *   Pointer to transaction structure.
 *
 * @return PARSING_OK if success, error status otherwise.
 *
 */
parser_status_e simple_transfer_deserialize(buffer_t *buf, transaction_ctx_t *tx);
