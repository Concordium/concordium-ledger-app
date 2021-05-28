#include "util.h"
#include <os.h>
#include <os_io_seproxyhal.h>
#include "cx.h"
#include <stdint.h>
#include <stdio.h>
#include "sign.h"

UX_STEP_NOCB(
    ux_sign_flow_account_sender_view,
    bn_paging,
    {
      .title = "Sender",
      .text = (char *) global_account_sender.sender
    });
    