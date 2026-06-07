#include "runtime_state.h"
#include "picoui/core.h"

#include <string.h>

static struct picoui_runtime_state g_picoui_runtime_state;

struct picoui_runtime_state *picoui_runtime_state(void)
{
    return &g_picoui_runtime_state;
}

int picoui_init(void)
{
    struct picoui_runtime_state *state = picoui_runtime_state();

    memset(state, 0, sizeof(*state));
    state->initialized = 1;
    state->default_screen.active = 1;
    state->active_screen = &state->default_screen;
    state->key = PICOUI_INPUT_KEY_NONE;
    return 0;
}

void picoui_deinit(void)
{
    struct picoui_runtime_state *state = picoui_runtime_state();

    memset(state, 0, sizeof(*state));
}

int picoui_timer_handler(void)
{
    if (!picoui_runtime_state()->initialized) {
        return -1;
    }

    return picoui_v1_1_render_active_screen();
}
