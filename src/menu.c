#include "menu.h"
#include "os.h"
#include "ux.h"
#include "globals.h"

UX_STEP_NOCB(
    ux_menu_idle_flow_1_step,
    bn,
    {
      "Concordium",
      "is ready",
    });
UX_STEP_VALID(
    ux_menu_idle_flow_2_step,
    pb,
    os_sched_exit(-1),
    {
      &C_icon_dashboard_x,
      "Quit",
    });
UX_FLOW(ux_menu_idle_flow,
  &ux_menu_idle_flow_1_step,
  &ux_menu_idle_flow_2_step,
  FLOW_LOOP
);

void ui_idle(void) {
    // reserve a display stack slot if none yet
    if (G_ux.stack_count == 0) {
        ux_stack_push();
    }
    ux_flow_init(0, ux_menu_idle_flow, NULL);
}