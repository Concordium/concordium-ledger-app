/*******************************************************************************
 *
 *   (c) 2016 Ledger
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
 ********************************************************************************/

#include <stdbool.h>
#include <string.h>

#include "os.h"
#include "cx.h"
#include "os_io_seproxyhal.h"
#include "ux.h"
#include "common/ui/menu.h"

// Boot procedure that ensures that the main menu is loaded, and that the application then starts
// listening for APDU commands by running concordium_main().
__attribute__((section(".boot"))) int main(void) {
    // exit critical section
    __asm volatile("cpsie i");

    os_boot();

    for (;;) {
        UX_INIT();

        BEGIN_TRY {
            TRY {
                io_seproxyhal_init();

#if defined(HAVE_BLE)
                G_io_app.plane_mode = os_setting_get(OS_SETTING_PLANEMODE, NULL, 0);
#endif

                USB_power(0);
                USB_power(1);
                ui_menu_main();

#if defined(HAVE_BLE)
                BLE_power(0, NULL);
                BLE_power(1, NULL);
#endif

                app_main();
            }
            CATCH(EXCEPTION_IO_RESET) {
                // reset IO and UX before continuing
                continue;
            }
            CATCH_ALL {
                break;
            }
            FINALLY {
            }
        }
        END_TRY;
    }
    return 0;
}
