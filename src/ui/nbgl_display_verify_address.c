/*****************************************************************************
 *   Ledger App Concordium.
 *   (c) 2020 Ledger SAS.
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

// TODO: IMPLEMENT THE WHOLE FLOW
// Buffer where the transaction amount string is written
static char g_verify_address_data[14];
// Buffer where the transaction address string is written
static char g_address[57];

static nbgl_layoutTagValue_t pairs[2];
static nbgl_layoutTagValueList_t pairList;


static void review_choice(bool confirm) {
    // Answer, display a status page and go back to main
    validate_verify_address(confirm);
    if (confirm) {
        nbgl_useCaseReviewStatus(STATUS_TYPE_ADDRESS_VERIFIED, ui_menu_main);
    } else {
        nbgl_useCaseReviewStatus(STATUS_TYPE_ADDRESS_REJECTED, ui_menu_main);
    }
}

int ui_display_verify_address() {
    if (G_context.req_type != CONFIRM_ADDRESS || G_context.state != STATE_NONE) {
        G_context.state = STATE_NONE;
        return io_send_sw(SW_BAD_STATE);
    }

    // Format the identity index and credential counter
    char identity_index[10];
    char credential_counter[10];
    snprintf(identity_index, sizeof(identity_index), "%u", G_context.verify_address_info.identity_index);
    snprintf(credential_counter, sizeof(credential_counter), "%u", G_context.verify_address_info.credential_counter);

    snprintf(g_verify_address_data,
            sizeof(g_verify_address_data),
            "%s/%s",
            identity_index,
            credential_counter);

    memset(g_address, 0, sizeof(g_address));

    memcpy(g_address, G_context.verify_address_info.address, sizeof(g_address));

    // Setup data to display
    pairs[0].item = "Verify Address";
    pairs[0].value = g_verify_address_data;
    pairs[1].item = "Address";
    pairs[1].value = g_address;

    // Setup list
    pairList.nbMaxLinesForValue = 0;
    pairList.nbPairs = 2;
    pairList.pairs = pairs;


    // Start review flow
    nbgl_useCaseReviewLight(TYPE_OPERATION,
                        &pairList,
                        &C_app_concordium_64px,
                        "Verify Address",
                        NULL,
                        "Verify Address",
                        review_choice);

    return 0;
}

#endif
