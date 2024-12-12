#pragma once

#include "os.h"

void concordium_main(
    void (*handler)(uint8_t, uint8_t *, uint8_t, uint8_t, uint8_t, volatile unsigned int *, bool),
    void *global_state);

// void app_exit(void);
