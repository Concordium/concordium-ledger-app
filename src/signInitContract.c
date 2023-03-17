#include <os.h>

#include "accountSenderView.h"
#include "responseCodes.h"
#include "sign.h"
#include "util.h"

static initContractContext_t *ctx = &global.signInitContract;
static tx_state_t *tx_state = &global_tx_state;

UX_STEP_NOCB(
    ux_sign_init_contract_display_amount,
    bnnn_paging,
    {.title = "Amount (CCD)", .text = (char *) global.signInitContract.displayAmount});

UX_STEP_NOCB(
    ux_sign_init_contract_display_module_reference,
    bnnn_paging,
    {.title = "Module Ref", .text = (char *) global.signInitContract.displayModuleRef});

UX_STEP_NOCB(
    ux_sign_init_contract_display_contract_name,
    bnnn_paging,
    {.title = "Contract name", .text = (char *) global.signInitContract.display});

UX_STEP_NOCB(
    ux_sign_init_contract_display_parameter,
    bnnn_paging,
    {.title = "Parameter", .text = (char *) global.signInitContract.display});

UX_STEP_NOCB(
    ux_sign_init_contract_display_no_parameter,
    bn,
    {"No Parameter", ""});

UX_STEP_VALID(ux_sign_init_contract_continue, nn, sendSuccessNoIdle(), {"Continue", "with transaction"});

// Display parameter with continue
UX_FLOW(
    ux_sign_init_contract_parameter,
    &ux_sign_init_contract_display_parameter,
    &ux_sign_init_contract_continue);
// Display parameter with finish
UX_FLOW(
    ux_sign_init_contract_finish,
    &ux_sign_init_contract_display_parameter,
    &ux_sign_flow_shared_sign,
    &ux_sign_flow_shared_decline);

const ux_flow_step_t *ux_sign_init_contract_flow[11];

/**
 * Builds and inits the ux flow for the contractName.
 * prepends the initial batch of info and appends continue/sign pages based on the given parameters.
 */
void startInitContractDisplayContractName(bool displayStart, bool finalContractName, bool emptyParameter) {
    uint8_t index = 0;
    if (displayStart) {
        ux_sign_init_contract_flow[index++] = &ux_sign_flow_shared_review;
        ux_sign_init_contract_flow[index++] = &ux_sign_flow_account_sender_view;
        ux_sign_init_contract_flow[index++] = &ux_sign_init_contract_display_amount;
        ux_sign_init_contract_flow[index++] = &ux_sign_init_contract_display_module_reference;
    }

    ux_sign_init_contract_flow[index++] = &ux_sign_init_contract_display_contract_name;

    if (finalContractName && emptyParameter) {
        ux_sign_init_contract_flow[index++] = &ux_sign_init_contract_display_no_parameter;
        ux_sign_init_contract_flow[index++] = &ux_sign_flow_shared_sign;
        ux_sign_init_contract_flow[index++] = &ux_sign_flow_shared_decline;
    } else {
        ux_sign_init_contract_flow[index++] = &ux_sign_init_contract_continue;
    }
    ux_sign_init_contract_flow[index++] = FLOW_END_STEP;
    ux_flow_init(0, ux_sign_init_contract_flow, NULL);
}


#define P1_INITIAL          0x00
#define P1_CONTRACT_NAME 0x01
#define P1_PARAMETER     0x02

void handleSignInitContract(uint8_t *cdata, uint8_t p1, uint8_t dataLength, volatile unsigned int *flags, bool isInitialCall) {
    if (isInitialCall) {
        ctx->state = TX_INIT_CONTRACT_INITIAL;
    }

    if (p1 == P1_INITIAL && ctx->state == TX_INIT_CONTRACT_INITIAL) {
        cdata += parseKeyDerivationPath(cdata);
        cx_sha256_init(&tx_state->hash);
        cdata += hashAccountTransactionHeaderAndKind(cdata, INIT_CONTRACT);

        // Build display value of the amount to transfer to the contract, and also add the bytes to the hash.
        uint64_t amount = U8BE(cdata, 0);
        amountToGtuDisplay(ctx->displayAmount, sizeof(ctx->displayAmount), amount);
        cx_hash((cx_hash_t *) &tx_state->hash, 0, cdata, 8, NULL, 0);
        cdata += 8;

        // Build display value of the reference for the module used for the contract, and also add the bytes to the hash.
        toPaginatedHex(cdata, 32, ctx->displayModuleRef, sizeof(ctx->displayModuleRef));
        cx_hash((cx_hash_t *) &tx_state->hash, 0, cdata, 32, NULL, 0);
        cdata += 32;

        // Save the length of the contractName and add the bytes to the hash
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
        ctx->state = TX_INIT_CONTRACT_CONTRACT_NAME;
        sendSuccessNoIdle();
    } else if (p1 == P1_CONTRACT_NAME && ctx->state == TX_INIT_CONTRACT_CONTRACT_NAME) {
        ctx->nameLength -= dataLength;
        cx_hash((cx_hash_t *) &tx_state->hash, 0, cdata, dataLength, NULL, 0);

        // Build display of contract name chunk
        memmove(ctx->display, cdata, dataLength);
        if (dataLength < 255) {
            memmove(ctx->display + dataLength, "\0", 1);
        }

        if (ctx->nameLength < 0) {
            THROW(ERROR_INVALID_STATE);
        } else if (ctx->nameLength == 0) {
            // Hash the parameterLength
            cx_hash((cx_hash_t *) &tx_state->hash, 0, ctx->rawParameterLength, 2, NULL, 0);
            ctx->state = TX_INIT_CONTRACT_PARAMETER;
        }

        startInitContractDisplayContractName(ctx->displayStart, ctx->nameLength == 0, ctx->paramLength == 0);

        if (ctx->displayStart) {
            ctx->displayStart = false;
        }
        *flags |= IO_ASYNCH_REPLY;
    } else if (p1 == P1_PARAMETER && ctx->state == TX_INIT_CONTRACT_PARAMETER) {
        ctx->paramLength -= dataLength;
        cx_hash((cx_hash_t *) &tx_state->hash, 0, cdata, dataLength, NULL, 0);

        // Build display of parameter chunk
        memmove(ctx->display, cdata, dataLength);
        if (dataLength < 255) {
            memmove(ctx->display + dataLength, "\0", 1);
        }

        if (ctx->paramLength < 0) {
            THROW(ERROR_INVALID_PARAM);
        } else if (ctx->paramLength == 0) {
            ctx->state = TX_INIT_CONTRACT_CONTRACT_NAME;

            ux_flow_init(0, ux_sign_init_contract_finish, NULL);
        } else {
            ux_flow_init(0, ux_sign_init_contract_parameter, NULL);
           sendSuccessNoIdle();
        }
        *flags |= IO_ASYNCH_REPLY;
    } else {
        THROW(ERROR_INVALID_STATE);
    }
}
