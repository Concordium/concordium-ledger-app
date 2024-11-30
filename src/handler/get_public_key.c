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

#include <stdint.h>   // uint*_t
#include <stdbool.h>  // bool
#include <stddef.h>   // size_t
#include <string.h>   // memset, explicit_bzero

#include "os.h"
#include "cx.h"
#include "io.h"
#include "buffer.h"
#include "crypto_helpers.h"

#include "get_public_key.h"
#include "../globals.h"
#include "../types.h"
#include "../sw.h"
#include "../ui/display.h"
#include "../helper/util.h"
#include "../helper/send_response.h"

int get_public_key(uint32_t *path, size_t path_len, uint8_t *public_key_array) {
    cx_ecfp_private_key_t private_key;
    cx_ecfp_public_key_t public_key;
    int rtn = get_private_key_from_path(path, path_len, &private_key);
    if (rtn != 0) {
        return rtn;
    }
    if (cx_ecfp_generate_pair_no_throw(CX_CURVE_Ed25519, &public_key, &private_key, 1) != CX_OK) {
        // Clear the private key
        explicit_bzero(&private_key, sizeof(private_key));
        return -3;
    }

    explicit_bzero(&private_key, sizeof(private_key));

    // Format the public key
    for (int i = 0; i < 32; i++) {
        public_key_array[i] = public_key.W[64 - i];
    }
    if ((public_key.W[32] & 1) != 0) {
        public_key_array[31] |= 0x80;
    }
    return 0;
}

int handler_get_public_key(buffer_t *cdata, bool display, bool sign_public_key) {
    explicit_bzero(&G_context, sizeof(G_context));
    G_context.req_type = CONFIRM_PUBLIC_KEY;
    G_context.state = STATE_NONE;

    if (!buffer_read_u8(cdata, &G_context.bip32_path_len) ||
        !buffer_read_bip32_path(cdata, G_context.bip32_path, (size_t) G_context.bip32_path_len)) {
        return io_send_sw(SW_WRONG_DATA_LENGTH);
    }
    int path_type = derivation_path_type(G_context.bip32_path, G_context.bip32_path_len);
    if (path_type <= 0) {
        PRINTF("ERROR: Invalid path type: %d\n", path_type);
        return io_send_sw(SW_INVALID_PATH);
    }
    harden_derivation_path(G_context.bip32_path, G_context.bip32_path_len);
    int rtn = get_public_key(G_context.bip32_path,
                             G_context.bip32_path_len,
                             G_context.pk_info.public_key);
    // Error handling
    switch (rtn) {
        case -1:
            // Derivation path failed
            return io_send_sw(SW_DERIVATION_PATH_FAIL);
        case -2:
            // Key initialization failed
            return io_send_sw(SW_KEY_INIT_FAIL);
        case -3:
            // Public key derivation failed
            return io_send_sw(SW_PUBLIC_KEY_DERIVATION_FAIL);
        case 0:
            break;
        default:
            // This should never happen
            return io_send_sw(SW_BAD_STATE);
    }

    if (sign_public_key) {
        G_context.pk_info.sign_public_key = true;
    }
    // Change the title of the public key depending on if signed or not or if it is a governance key
    char temp_title[36];
    size_t offset = 0;
    strncpy(temp_title, "Get ", 4);
    offset += 4;

    // Append "Signed" to the title if needed
    if (sign_public_key) {
        strncpy(temp_title + offset, "Signed ", 7);
        offset += 7;
    }

    // Append "Gov." to the title if needed
    if (path_type >= 10) {
        strncpy(temp_title + offset, "Gov. ", 5);
        offset += 5;
        // Get the purpose of the key
        // TODO: enter the correct index for new path in constants.h and add a check for it
        uint32_t purpose = G_context.bip32_path[LEGACY_PATH_PURPOSE_INDEX];
        // Change the title of the public key depending on the purpose
        switch (purpose) {
            case 0 | HARDENED_OFFSET:
                strncpy(temp_title + offset, "Root ", 5);
                offset += 5;
                break;
            case 1 | HARDENED_OFFSET:
                strncpy(temp_title + offset, "Level 1 ", 7);
                offset += 7;
                break;
            case 2 | HARDENED_OFFSET:
                strncpy(temp_title + offset, "Level 2 ", 7);
                offset += 7;
                break;
            default:
                PRINTF("Unknown purpose: %lx\n", purpose);
                return io_send_sw(SW_INVALID_PATH);
        }
    }

    // Append "Public Key" to the title
    strncpy(temp_title + offset, "Public Key", 11);
    offset += 11;
    // Add null terminator
    temp_title[offset] = '\0';
    strncpy(G_context.pk_info.public_key_title, temp_title, 36);

    if (display) {
        return ui_display_pubkey();
    }

    return helper_send_response_pubkey();
}
