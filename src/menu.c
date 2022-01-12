#include "globals.h"
#include "util.h"
#include "ux.h"

static tx_state_t *tx_state = &global_tx_state;
uint8_t version[10];

/**
 * Constructs the version in the format MAJOR.MINOR.PATCH. We only
 * reserve 10 bytes to hold the version, so the maximum version
 * supported is xx.yy.zzz.
 */
void loadVersion(uint8_t *version) {
    int majorLength = numberToText(version, 2, APPVERSION_MAJOR);
    version[majorLength] = '.';
    int minorLength = numberToText(version + majorLength + 1, 2, APPVERSION_MINOR);
    version[majorLength + minorLength + 1] = '.';
    numberToText(version + majorLength + minorLength + 2, 3, APPVERSION_PATCH);
    version[9] = '\0';
}

UX_STEP_NOCB(
    ux_menu_idle_flow_1_step,
    bn,
    {
        "Concordium",
        "is ready",
    });
UX_STEP_CB(
    ux_menu_idle_flow_2_step,
    pb,
    os_sched_exit(-1),
    {
        &C_icon_dashboard_x,
        "Quit",
    });
UX_STEP_NOCB(
    ux_menu_idle_flow_3_step,
    bn,
    {
        "Version",
        (char *) version,
    });
UX_FLOW(ux_menu_idle_flow, &ux_menu_idle_flow_1_step, &ux_menu_idle_flow_2_step, &ux_menu_idle_flow_3_step, FLOW_LOOP);

void ui_idle(void) {
    tx_state->currentInstruction = -1;
    if (G_ux.stack_count == 0) {
        ux_stack_push();
    }
    loadVersion(version);
    ux_flow_init(0, ux_menu_idle_flow, NULL);
}
