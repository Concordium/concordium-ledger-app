#ifdef HAVE_NBGL

#include "os.h"
#include "glyphs.h"
#include "nbgl_use_case.h"
#include "menu.h"

void app_quit(void) {
    // exit app here
    os_sched_exit(-1);
}


static const char* const INFO_TYPES[] = {"Version", "Developer"};
static const char* const INFO_CONTENTS[] = {"Hello", "Ledger"};


static bool nav_callback(uint8_t page, nbgl_pageContent_t* content) {
    UNUSED(page);
    content->type = INFOS_LIST;
    content->infosList.nbInfos = 2;
    content->infosList.infoTypes = (const char**) INFO_TYPES;
    content->infosList.infoContents = (const char**) INFO_CONTENTS;
    return true;
}

void ui_menu_about() {
    nbgl_useCaseSettings("Test", 0, 1, false, ui_idle, nav_callback, NULL);
}

void ui_idle(void) {
    nbgl_useCaseHome("Test", &C_stax_app_boilerplate_64px, NULL, false, ui_menu_about, app_quit);
}

#endif
