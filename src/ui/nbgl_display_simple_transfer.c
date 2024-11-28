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
#include "os_io_seproxyhal.h"
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
#include "../helper/util.h"

// Buffer where the transaction amount string is written
static char g_amount[30];
// Buffer where the transaction recipient address string is written
static char g_recipient_address[57];
// Buffer where the transaction sender address string is written
static char g_sender_address[57];

// Array of pairs to display
static nbgl_layoutTagValue_t pairs[3];
static nbgl_layoutTagValueList_t pairList;

// called when long press button on 3rd page is long-touched or when reject footer is touched
static void review_choice(bool confirm) {
    // Answer, display a status page and go back to main
    validate_transaction(confirm);
    if (confirm) {
        nbgl_useCaseReviewStatus(STATUS_TYPE_TRANSACTION_SIGNED, ui_menu_main);
    } else {
        nbgl_useCaseReviewStatus(STATUS_TYPE_TRANSACTION_REJECTED, ui_menu_main);
    }
}


// Flow used to display a clear-signed transaction

// Public function to start the transaction review
// - Check if the app is in the right state for transaction review
// - Format the amount and address strings in g_amount and g_address buffers
// - Display the first screen of the transaction review
int ui_display_simple_transfer() {

    if (G_context.req_type != CONFIRM_TRANSACTION || G_context.state != STATE_PARSED) {
        G_context.state = STATE_NONE;
        return io_send_sw(SW_BAD_STATE);
    }

    // Format amount and address to g_amount and g_address buffers
    memset(g_amount, 0, sizeof(g_amount));
    char amount[30] = {0};
    if (!format_fpu64(amount,
                      sizeof(amount),
                      G_context.tx_info.transaction.simple_transfer.value,
                      EXPONENT_SMALLEST_UNIT)) {
        return io_send_sw(SW_DISPLAY_AMOUNT_FAIL);
    }
    snprintf(g_amount, sizeof(g_amount), "CCD %.*s", sizeof(amount), amount);
    memset(g_recipient_address, 0, sizeof(g_recipient_address));
    memset(g_sender_address, 0, sizeof(g_sender_address));

     // Format the recipient address
    if(address_to_base58(G_context.tx_info.transaction.simple_transfer.recipient, ADDRESS_LEN, g_recipient_address, sizeof(g_recipient_address)) == -1){
        return io_send_sw(SW_DISPLAY_ADDRESS_FAIL);
    }
    // Format the sender address
    if(address_to_base58(G_context.tx_info.transaction.simple_transfer.sender, ADDRESS_LEN, g_sender_address, sizeof(g_sender_address)) == -1){
        return io_send_sw(SW_DISPLAY_ADDRESS_FAIL);
    }

    // Setup data to display
    pairs[0].item = "Sender";
    pairs[0].value = g_sender_address;
    pairs[1].item = "Amount";
    pairs[1].value = g_amount;
    pairs[2].item = "Recipient";
    pairs[2].value = g_recipient_address;

    // Setup list
    pairList.nbMaxLinesForValue = 0;
    pairList.nbPairs = 3;
    pairList.pairs = pairs;

    // Start review flow
    nbgl_useCaseReview(TYPE_TRANSACTION,
                        &pairList,
                        &C_app_concordium_64px,
                        "Review\nSimple transfer",
                        NULL,
                        "Sign\nSimple transfer",
                        review_choice);

    return 0;
}

#endif
