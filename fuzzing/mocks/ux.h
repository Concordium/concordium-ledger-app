#pragma once

// Mock UX types and globals needed for fuzzing
typedef struct {
    unsigned int dummy;
} ux_state_t;

#define G_ux_params (dummy_ux_state)

extern ux_state_t dummy_ux_state;