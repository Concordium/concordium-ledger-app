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

#ifdef HAVE_BAGL

#include <stdbool.h>  // bool
#include <string.h>   // memset

#include "os.h"
#include "ux.h"
#include "glyphs.h"
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


static action_validate_cb g_validate_callback;
static char g_amount[30];
static char g_sender_address[57];
static char g_verify_address_data[21];
static char g_recipient_address[57];
static char g_public_key[65];

// Validate/Invalidate public key and go back to home
static void ui_action_validate_pubkey(bool choice) {
    validate_pubkey(choice);
    ui_menu_main();
}

// Validate/Invalidate verify address and go back to home
static void ui_action_validate_verify_address(bool choice) {
    validate_verify_address(choice);
    ui_menu_main();
}

// Validate/Invalidate transaction and go back to home
static void ui_action_validate_transaction(bool choice) {
    validate_transaction(choice);
    ui_menu_main();
}

// Step with title/text for identity index and credential counter
UX_STEP_NOCB(ux_verify_address_0_step,
             bnnn_paging,
             {.title = "Verify Address", .text = g_verify_address_data});

// Step with title/text for address
UX_STEP_NOCB(ux_verify_address_1_step, bnnn_paging, {.title = "Address", .text = g_sender_address});
// Step with approve button
UX_STEP_CB(ux_display_approve_step,
           pb,
           (*g_validate_callback)(true),
           {
               &C_icon_validate_14,
               "Approve",
           });
// Step with reject button
UX_STEP_CB(ux_display_reject_step,
           pb,
           (*g_validate_callback)(false),
           {
               &C_icon_crossmark,
               "Reject",
           });
UX_FLOW(ux_display_verify_address_flow,
        &ux_verify_address_0_step,
        &ux_verify_address_1_step,
        &ux_display_approve_step,
        &ux_display_reject_step);

int ui_display_verify_address() {
    if (G_context.req_type != CONFIRM_ADDRESS || G_context.state != STATE_NONE) {
        G_context.state = STATE_NONE;
        return io_send_sw(SW_BAD_STATE);
    }
    char idp_index[10];
    char identity_index[10];
    char credential_counter[10];

    char temp_data[21] = {0};

    if (G_context.verify_address_info.idp_index != 0xffffffff) {
        // Format the idp index
        snprintf(idp_index, sizeof(idp_index), "%u", G_context.verify_address_info.idp_index);
        // Prepend the idp index to the address data
        strncpy(temp_data, idp_index, sizeof(temp_data) - 1);
        if (strlen(temp_data) < sizeof(temp_data) - 1) {
            strncat(temp_data, "/", sizeof(temp_data) - strlen(temp_data) - 1);
        }
    }

    // Turn the uint32_t values into strings
    snprintf(identity_index,
             sizeof(identity_index),
             "%u",
             G_context.verify_address_info.identity_index);
    snprintf(credential_counter,
             sizeof(credential_counter),
             "%u",
             G_context.verify_address_info.credential_counter);

    if (strlen(temp_data) == 0) {
        strncpy(temp_data, identity_index, sizeof(temp_data) - 1);
    } else {
        strncat(temp_data, identity_index, sizeof(temp_data) - strlen(temp_data) - 1);
    }
    strncat(temp_data, "/", sizeof(temp_data) - strlen(temp_data) - 1);
    strncat(temp_data, credential_counter, sizeof(temp_data) - strlen(temp_data) - 1);

    memcpy(g_verify_address_data, temp_data, strlen(temp_data));

    memset(g_sender_address, 0, sizeof(g_sender_address));

    memcpy(g_sender_address, G_context.verify_address_info.address, sizeof(g_sender_address));

    g_validate_callback = &ui_action_validate_verify_address;

    ux_flow_init(0, ux_display_verify_address_flow, NULL);

    return 0;
}

// Step with icon and text
UX_STEP_NOCB(ux_display_review_step,
             pnn,
             {
                 &C_icon_eye,
                 "Review",
                 "Transaction",
             });
// Step with title/text for amount
UX_STEP_NOCB(ux_display_amount_step,
             bnnn_paging,
             {
                 .title = "Amount",
                 .text = g_amount,
             });

// Step with icon and text
UX_STEP_NOCB(ux_display_review_simple_transfer_step,
             pnn,
             {
                 &C_icon_eye,
                 "Review",
                 "Simple Transfer",
             });

// Step with title/text for sender address
UX_STEP_NOCB(ux_display_sender_address_step,
             bnnn_paging,
             {
                 .title = "Sender",
                 .text = g_sender_address,
             });
// Step with title/text for recipient address
UX_STEP_NOCB(ux_display_recipient_address_step,
             bnnn_paging,
             {
                 .title = "Recipient",
                 .text = g_recipient_address,
             });

// FLOW to display transaction information:
// #1 screen : eye icon + "Review Transaction"
// #2 screen : display amount
// #3 screen : display recipient address
// #4 screen : approve button
// #5 screen : reject button
UX_FLOW(ux_display_simple_transfer_flow,
        &ux_display_review_simple_transfer_step,
        &ux_display_sender_address_step,
        &ux_display_amount_step,
        &ux_display_recipient_address_step,
        &ux_display_approve_step,
        &ux_display_reject_step);
int ui_display_simple_transfer() {
    if (G_context.req_type != CONFIRM_TRANSACTION || G_context.state != STATE_PARSED) {
        G_context.state = STATE_NONE;
        return io_send_sw(SW_BAD_STATE);
    }

    // Format the amount
    memset(g_amount, 0, sizeof(g_amount));
    char amount[30] = {0};
    if (!format_fpu64(amount,
                      sizeof(amount),
                      G_context.tx_info.transaction.simple_transfer.value,
                      EXPONENT_SMALLEST_UNIT)) {
        return io_send_sw(SW_DISPLAY_AMOUNT_FAIL);
    }
    snprintf(g_amount, sizeof(g_amount), "CCD %.*s", sizeof(amount), amount);

    // Format the recipient address
    if (address_to_base58(G_context.tx_info.transaction.simple_transfer.recipient,
                          ADDRESS_LEN,
                          g_recipient_address,
                          sizeof(g_recipient_address)) == -1) {
        return io_send_sw(SW_DISPLAY_ADDRESS_FAIL);
    }
    // Format the sender address
    if (address_to_base58(G_context.tx_info.transaction.simple_transfer.sender,
                          ADDRESS_LEN,
                          g_sender_address,
                          sizeof(g_sender_address)) == -1) {
        return io_send_sw(SW_DISPLAY_ADDRESS_FAIL);
    }

    g_validate_callback = &ui_action_validate_transaction;

    ux_flow_init(0, ux_display_simple_transfer_flow, NULL);

    return 0;
}

// Step with icon and text
UX_STEP_NOCB(ux_display_confirm_public_key_step, pn, {&C_icon_eye, "Confirm Public Key"});
// Step with title/text for address
UX_STEP_NOCB(ux_display_public_key_step,
             bnnn_paging,
             {
                 .title = "Public Key",
                 .text = g_public_key,
             });
// FLOW to display transaction information:
// #1 screen : eye icon + "Review Transaction"
// #2 screen : display amount
// #3 screen : display recipient address
// #4 screen : approve button
// #5 screen : reject button
UX_FLOW(ux_display_public_key_flow,
        &ux_display_confirm_public_key_step,
        &ux_display_public_key_step,
        &ux_display_approve_step,
        &ux_display_reject_step);
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

    g_validate_callback = &ui_action_validate_pubkey;
    ux_flow_init(0, ux_display_public_key_flow, NULL);
    return 0;

    // memset(g_address, 0, sizeof(g_address));
    // uint8_t address[ADDRESS_LEN] = {0};
    // if (!address_from_pubkey(G_context.pk_info.raw_public_key, address, sizeof(address))) {
    //     return io_send_sw(SW_DISPLAY_ADDRESS_FAIL);
    // }

    // if (format_hex(address, sizeof(address), g_address, sizeof(g_address)) == -1) {
    //     return io_send_sw(SW_DISPLAY_ADDRESS_FAIL);
    // }

    // g_validate_callback = &ui_action_validate_pubkey;
    // ux_flow_init(0, ux_display_pubkey_flow, NULL);
    // return 0;
}
#endif
