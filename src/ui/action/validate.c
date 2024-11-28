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

#include <stdbool.h>  // bool

#include "crypto_helpers.h"

#include "validate.h"
#include "cx.h"
#include "../menu.h"
#include "../../sw.h"
#include "../../globals.h"
#include "../../helper/util.h"
#include "../../helper/send_response.h"

void validate_pubkey(bool choice) {
    if (choice) {
        helper_send_response_pubkey();
    } else {
        io_send_sw(SW_DENY);
    }
}
void validate_verify_address(bool choice) {
    if (choice) {
        io_send_sw(SW_OK);
    } else {
        io_send_sw(SW_DENY);
    }
}

static int crypto_sign_message(void) {
    size_t sig_len = sizeof(G_context.tx_info.signature);
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
    error = cx_eddsa_sign_no_throw(&private_key,
                                   CX_SHA512,
                                   G_context.tx_info.m_hash,
                                   sizeof(G_context.tx_info.m_hash),
                                   G_context.tx_info.signature,
                                   sig_len);

    if (error != CX_OK) {
        return -1;
    }

    PRINTF("Signature: %.*H\n", sig_len, G_context.tx_info.signature);

    G_context.tx_info.signature_len = sig_len;

    return 0;
}

void validate_transaction(bool choice) {
    if (choice) {
        G_context.state = STATE_APPROVED;

        if (crypto_sign_message() != 0) {
            G_context.state = STATE_NONE;
            io_send_sw(SW_SIGNATURE_FAIL);
        } else {
            helper_send_response_sig();
        }
    } else {
        G_context.state = STATE_NONE;
        io_send_sw(SW_DENY);
    }
}
