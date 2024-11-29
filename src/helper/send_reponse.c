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

#include <stddef.h>  // size_t
#include <stdint.h>  // uint*_t
#include <string.h>  // memmove

#include "buffer.h"

#include "send_response.h"
#include "../constants.h"
#include "../globals.h"
#include "../sw.h"
#include "../helper/util.h"

// TODO: fix this, something is wrong here, the signatures don't match
int helper_send_response_pubkey() {

    uint8_t resp[1 + PUBKEY_LEN + 1 + MAX_DER_SIG_LEN] = {0};
    size_t offset = 0;

    // Add public key to response
    resp[offset++] = PUBKEY_LEN;
    memmove(resp + offset, G_context.pk_info.public_key, PUBKEY_LEN);
    offset += PUBKEY_LEN;

    // Sign public key if requested and add to response
    if (G_context.pk_info.sign_public_key) {
        if (sign(G_context.pk_info.m_hash,
                 sizeof(G_context.pk_info.m_hash),
                 G_context.pk_info.signature,
                 MAX_DER_SIG_LEN) != 0) {
            return io_send_sw(SW_SIGNATURE_FAIL);
        }

        resp[offset++] = MAX_DER_SIG_LEN;
        memmove(resp + offset, G_context.pk_info.signature, MAX_DER_SIG_LEN);
        offset += MAX_DER_SIG_LEN;
    }

    return io_send_response_pointer(resp, offset, SW_OK);
}

int helper_send_response_sig() {
    return io_send_response_pointer(G_context.tx_info.signature,
                                    G_context.tx_info.signature_len,
                                    SW_OK);
}
