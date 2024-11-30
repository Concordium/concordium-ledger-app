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

#ifdef HAVE_NBGL

#include <stdbool.h>  // bool
#include <string.h>   // memset

#include "os.h"
#include "glyphs.h"
#include "nbgl_use_case.h"
#include "io.h"
#include "bip32.h"
#include "format.h"

#include "display.h"
#include "constants.h"
#include "../globals.h"
#include "../sw.h"
#include "../address.h"
#include "action/validate.h"
#include "../transaction/types.h"
#include "../menu.h"

static char g_public_key[PUBKEY_LEN * 2 + 1];
static char g_bip32_path_string[MAX_SERIALIZED_BIP32_PATH_LENGTH + 1];

static nbgl_layoutTagValue_t pairs[2];
static nbgl_layoutTagValueList_t pairList;

static void review_choice(bool confirm) {
    // Answer, display a status page and go back to main
    validate_pubkey(confirm);
    if (confirm) {
        nbgl_useCaseReviewStatus(STATUS_TYPE_ADDRESS_VERIFIED, ui_menu_main);
    } else {
        nbgl_useCaseReviewStatus(STATUS_TYPE_ADDRESS_REJECTED, ui_menu_main);
    }
}

int ui_display_pubkey() {
    if (G_context.req_type != CONFIRM_PUBLIC_KEY || G_context.state != STATE_NONE) {
        G_context.state = STATE_NONE;
        return io_send_sw(SW_BAD_STATE);
    }

    // Format the public key
    if (format_hex(G_context.pk_info.public_key, PUBKEY_LEN, g_public_key, sizeof(g_public_key)) ==
        -1) {
        return io_send_sw(SW_PUBLIC_KEY_DISPLAY_FAIL);
    }

    bip32_path_format(G_context.bip32_path,
                      G_context.bip32_path_len,
                      g_bip32_path_string,
                      sizeof(g_bip32_path_string));

    // Setup data to display
    pairs[0].item = "BIP32 Path";
    pairs[0].value = g_bip32_path_string;
    pairs[1].item = "Public Key";
    pairs[1].value = g_public_key;

    // Setup list
    pairList.nbMaxLinesForValue = 0;
    pairList.nbPairs = 2;
    pairList.pairs = pairs;

    // Start review flow
    nbgl_useCaseReviewLight(TYPE_OPERATION,
                            &pairList,
                            &C_app_concordium_64px,
                            "Verify Public Key",
                            NULL,
                            "Verify Public Key",
                            review_choice);

    return 0;
}

#endif
