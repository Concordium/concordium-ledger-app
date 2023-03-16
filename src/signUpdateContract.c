#include <os.h>

#include "accountSenderView.h"
#include "responseCodes.h"
#include "sign.h"
#include "util.h"

static updateContractContext_t *ctx = &global.signUpdateContract;
static tx_state_t *tx_state = &global_tx_state;

UX_STEP_NOCB(
    ux_sign_update_contract_display_amount,
    bnnn_paging,
    {.title = "Amount (CCD)", .text = (char *) global.signUpdateContract.displayAmount});

UX_STEP_NOCB(
    ux_sign_update_contract_display_index,
    bnnn_paging,
    {.title = "Contract index", .text = (char *) global.signUpdateContract.displayIndex});

UX_STEP_NOCB(
    ux_sign_update_contract_display_subindex,
    bnnn_paging,
    {.title = "Contract subindex", .text = (char *) global.signUpdateContract.displaySubindex});

UX_STEP_NOCB(
    ux_sign_update_contract_display_receive_name,
    bnnn_paging,
    {.title = "Receive name", .text = (char *) global.signUpdateContract.display});

UX_STEP_NOCB(
    ux_sign_update_contract_display_parameter,
    bnnn_paging,
    {.title = "Parameter", .text = (char *) global.signUpdateContract.display});

UX_STEP_NOCB(
    ux_sign_update_contract_display_no_parameter,
    bn,
    {"No Parameter", ""});

UX_STEP_VALID(ux_sign_update_contract_continue, nn, sendSuccessNoIdle(), {"Continue", "with transaction"});

// Display parameter with continue
UX_FLOW(
    ux_sign_update_contract_parameter,
    &ux_sign_update_contract_display_parameter,
    &ux_sign_update_contract_continue);
// Display parameter with finish
UX_FLOW(
    ux_sign_update_contract_finish,
    &ux_sign_update_contract_display_parameter,
    &ux_sign_flow_shared_sign,
    &ux_sign_flow_shared_decline);

const ux_flow_step_t *ux_sign_update_contract_flow[12];

void startUpdateContractDisplayReceiveName(bool displayStart, bool finalReceiveName, bool emptyParameter) {
    uint8_t index = 0;
    if (displayStart) {
        ux_sign_update_contract_flow[index++] = &ux_sign_flow_shared_review;
        ux_sign_update_contract_flow[index++] = &ux_sign_flow_account_sender_view;
        ux_sign_update_contract_flow[index++] = &ux_sign_update_contract_display_amount;
        ux_sign_update_contract_flow[index++] = &ux_sign_update_contract_display_index;
        ux_sign_update_contract_flow[index++] = &ux_sign_update_contract_display_subindex;
    }

    ux_sign_update_contract_flow[index++] = &ux_sign_update_contract_display_receive_name;

    if (finalReceiveName && emptyParameter) {
        ux_sign_update_contract_flow[index++] = &ux_sign_update_contract_display_no_parameter;
        ux_sign_update_contract_flow[index++] = &ux_sign_flow_shared_sign;
        ux_sign_update_contract_flow[index++] = &ux_sign_flow_shared_decline;
    } else {
        ux_sign_update_contract_flow[index++] = &ux_sign_update_contract_continue;
    }
    ux_sign_update_contract_flow[index++] = FLOW_END_STEP;
    ux_flow_init(0, ux_sign_update_contract_flow, NULL);
}


#define P1_INITIAL          0x00
#define P1_RECEIVE_NAME 0x01
#define P1_PARAMETER     0x02

void handleSignUpdateContract(uint8_t *cdata, uint8_t p1, uint8_t dataLength, volatile unsigned int *flags, bool isInitialCall) {
    if (isInitialCall) {
        ctx->state = TX_UPDATE_CONTRACT_INITIAL;
    }

    if (p1 == P1_INITIAL && ctx->state == TX_UPDATE_CONTRACT_INITIAL) {
        cdata += parseKeyDerivationPath(cdata);
        cx_sha256_init(&tx_state->hash);
        cdata += hashAccountTransactionHeaderAndKind(cdata, UPDATE);

        // Build display value of the amount to transfer to the contract, and also add the bytes to the hash.
        uint64_t amount = U8BE(cdata, 0);
        amountToGtuDisplay(ctx->displayAmount, sizeof(ctx->displayAmount), amount);
        cx_hash((cx_hash_t *) &tx_state->hash, 0, cdata, 8, NULL, 0);
        cdata += 8;

        // Build display value of the index of the contract, and also add the bytes to the hash.
        uint64_t index = U8BE(cdata, 0);
        amountToGtuDisplay(ctx->displayIndex, sizeof(ctx->displayIndex), index);
        cx_hash((cx_hash_t *) &tx_state->hash, 0, cdata, 8, NULL, 0);
        cdata += 8;

        // Build display value of the subindex of the contract, and also add the bytes to the hash.
        uint64_t subindex = U8BE(cdata, 0);
        amountToGtuDisplay(ctx->displaySubindex, sizeof(ctx->displaySubindex), subindex);
        cx_hash((cx_hash_t *) &tx_state->hash, 0, cdata, 8, NULL, 0);
        cdata += 8;

        // Save the length of the receiveName and add the bytes to the hash
        ctx->nameLength = U2BE(cdata, 0);
        cx_hash((cx_hash_t *) &tx_state->hash, 0, cdata, 2, NULL, 0);
        cdata += 2;

        // Save the length of the parameters
        ctx->paramLength = U2BE(cdata, 0);
        memmove(ctx->rawParameterLength, cdata, 2);

        if (ctx->nameLength == 0) {
            THROW(ERROR_INVALID_PARAM);
        }

        ctx->displayStart = true;
        ctx->state = TX_UPDATE_CONTRACT_RECEIVE_NAME;
        sendSuccessNoIdle();
    } else if (p1 == P1_RECEIVE_NAME && ctx->state == TX_UPDATE_CONTRACT_RECEIVE_NAME) {
        ctx->nameLength -= dataLength;
        cx_hash((cx_hash_t *) &tx_state->hash, 0, cdata, dataLength, NULL, 0);
        memmove(ctx->display, cdata, dataLength);
        if (dataLength < 255) {
            memmove(ctx->display + dataLength, "\0", 1);
        }

        if (ctx->nameLength < 0) {
            THROW(ERROR_INVALID_STATE);
        } else if (ctx->nameLength == 0) {
            // Hash the parameterLength
            cx_hash((cx_hash_t *) &tx_state->hash, 0, ctx->rawParameterLength, 2, NULL, 0);
            ctx->state = TX_UPDATE_CONTRACT_PARAMETER;
        }

        startUpdateContractDisplayReceiveName(ctx->displayStart, ctx->nameLength == 0, ctx->paramLength == 0);

        if (ctx->displayStart) {
            ctx->displayStart = false;
        }
        *flags |= IO_ASYNCH_REPLY;
    } else if (p1 == P1_PARAMETER && ctx->state == TX_UPDATE_CONTRACT_PARAMETER) {
        ctx->paramLength -= dataLength;
        cx_hash((cx_hash_t *) &tx_state->hash, 0, cdata, dataLength, NULL, 0);
        memmove(ctx->display, cdata, dataLength);
        if (dataLength < 255) {
            memmove(ctx->display + dataLength, "\0", 1);
        }

        if (ctx->paramLength < 0) {
            THROW(ERROR_INVALID_PARAM);
        } else if (ctx->paramLength == 0) {
            ctx->state = TX_UPDATE_CONTRACT_RECEIVE_NAME;

            ux_flow_init(0, ux_sign_update_contract_finish, NULL);
        } else {
            ux_flow_init(0, ux_sign_update_contract_parameter, NULL);
           sendSuccessNoIdle();
        }
        *flags |= IO_ASYNCH_REPLY;
    } else {
        THROW(ERROR_INVALID_STATE);
    }
}

