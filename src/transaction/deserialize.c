/*****************************************************************************
 *   Ledger App Concordium.
 *   (c) 2024 Concordium.
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *****************************************************************************/
#include "buffer.h"

#include "deserialize.h"
#include "utils.h"
#include "types.h"

#if defined(TEST) || defined(FUZZ)
#include "assert.h"
#define LEDGER_ASSERT(x, y) assert(x)
#else
#include "ledger_assert.h"
#endif

// TODO: EDIT THIS FUNCTION
parser_status_e simple_transfer_deserialize(buffer_t *buf, transaction_ctx_t *tx) {
    LEDGER_ASSERT(buf != NULL, "NULL buf");
    LEDGER_ASSERT(tx != NULL, "NULL tx");

    if (buf->size > MAX_TX_LEN) {
        return WRONG_LENGTH_ERROR;
    }

    // Sender address (32 bytes)
    tx->transaction.simple_transfer.sender = (uint8_t *) (buf->ptr + buf->offset);
    if (!buffer_seek_cur(buf, ADDRESS_LEN)) {
        return SENDER_PARSING_ERROR;
    }

    // Skip sequence number (8 bytes)
    if (!buffer_seek_cur(buf, 8)) {
        return PARSING_ERROR;
    }

    // Skip energy allowance (8 bytes)
    if (!buffer_seek_cur(buf, 8)) {
        return PARSING_ERROR;
    }

    // Skip payload size (4 bytes)
    if (!buffer_seek_cur(buf, 4)) {
        return PARSING_ERROR;
    }

    // Skip expiration (8 bytes)
    if (!buffer_seek_cur(buf, 8)) {
        return PARSING_ERROR;
    }

    // Transaction type (1 byte)
    if (!buffer_read_u8(buf, &tx->type)) {
        return TYPE_PARSING_ERROR;
    }

    // Recipient address (32 bytes)
    tx->transaction.simple_transfer.recipient = (uint8_t *) (buf->ptr + buf->offset);
    if (!buffer_seek_cur(buf, ADDRESS_LEN)) {
        return RECIPIENT_PARSING_ERROR;
    }

    // Amount (8 bytes)
    if (!buffer_read_u64(buf, &tx->transaction.simple_transfer.value, BE)) {
        return AMOUNT_PARSING_ERROR;
    }

    return PARSING_OK;
}
