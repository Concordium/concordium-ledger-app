#include <os.h>
#include "util.h"
#include "responseCodes.h"

static descriptionContext_t *ctx = &global.withDescription.descriptionContext;

void handleDescriptionPart();

UX_STEP_CB(
    ux_sign_add_identity_provider_name_step,
    bnnn_paging,
    handleDescriptionPart(),
    {
        "Name",
            (char *) global.withDescription.descriptionContext.text
            });
UX_FLOW(ux_sign_description_name,
        &ux_sign_add_identity_provider_name_step
    );

UX_STEP_CB(
    ux_sign_add_identity_provider_url_step,
    bnnn_paging,
    handleDescriptionPart(),
    {
        "URL",
            (char *) global.withDescription.descriptionContext.text
            });
UX_FLOW(ux_sign_description_url,
        &ux_sign_add_identity_provider_url_step
    );

UX_STEP_CB(
    ux_sign_add_identity_provider_description_step,
    bnnn_paging,
    handleDescriptionPart(),
    {
        "Description",
            (char *) global.withDescription.descriptionContext.text
            });
UX_FLOW(ux_sign_description_description,
        &ux_sign_add_identity_provider_description_step
    );

void handleDescriptionPart(void) {
    if (ctx->textLength == 0) {
        switch (ctx->descriptionState) {
        case NAME:
            ctx->descriptionState = URL;
            break;
        case URL:
            ctx->descriptionState = DESCRIPTION;
            break;
        case DESCRIPTION:
            break;
        default:
            THROW(ERROR_INVALID_STATE);
            break;
        }
    }
    sendSuccessNoIdle();
}
