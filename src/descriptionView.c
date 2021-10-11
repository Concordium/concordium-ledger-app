#include "ux.h"
#include "globals.h"
#include "util.h"
#include "descriptionView.h"
#include "responseCodes.h"

static descriptionContext_t *ctx = &global.withDescription.descriptionContext;

void handleDescriptionPart(void);

UX_STEP_CB(
    ux_sign_description_step,
    bnnn_paging,
    handleDescriptionPart(),
    {
        (char *) global.withDescription.descriptionContext.header,
            (char *) global.withDescription.descriptionContext.text
            });
UX_FLOW(ux_sign_description,
        &ux_sign_description_step
    );

void handleDescriptionPart(void) {
    if (ctx->textLength == 0) {
        switch (ctx->descriptionState) {
            case DESC_NAME:
                ctx->descriptionState = DESC_URL;
                break;
            case DESC_URL:
                ctx->descriptionState = DESC_DESCRIPTION;
                break;
            case DESC_DESCRIPTION:
                ctx->descriptionState = DESC_DESCRIPTION;
                break;
            default:
                THROW(ERROR_INVALID_STATE);
                break;
        }
    }
    sendSuccessNoIdle();
}

void displayDescriptionPart(volatile unsigned int *flags) {
    switch (ctx->descriptionState) {
        case DESC_NAME:
            memmove(ctx->header, "Name", 4);
            ctx->header[4] = '\0';
            break;
        case DESC_URL:
            memmove(ctx->header, "URL", 3);
            ctx->header[3] = '\0';
            break;
        case DESC_DESCRIPTION:
            memmove(ctx->header, "Description", 11);
            ctx->header[11] = '\0';
            break;
        default:
            THROW(ERROR_INVALID_STATE);
            break;
    }
    ux_flow_init(0, ux_sign_description, NULL);
    *flags |= IO_ASYNCH_REPLY;
}
