#include "ux.h"
#include "globals.h"

UX_STEP_NOCB(
    ux_sign_flow_account_sender_view,
    bnnn_paging,
    {
      .title = "Sender",
      .text = (char *) global_account_sender.sender
    });
