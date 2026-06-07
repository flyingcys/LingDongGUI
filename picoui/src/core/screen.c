#include "runtime_state.h"
#include "picoui/screen.h"

#include <stdlib.h>

struct picoui_screen {
    int loaded;
    struct picoui_window *root_window;
};

static struct picoui_screen g_picoui_v1_1_default_screen;

struct picoui_screen *picoui_screen_active(void)
{
    struct picoui_runtime_state *state = picoui_runtime_state();

    if (state->active_screen == 0) {
        state->active_screen = &g_picoui_v1_1_default_screen;
    }
    return state->active_screen;
}

struct picoui_screen *picoui_screen_create(void)
{
    return (struct picoui_screen *)calloc(1, sizeof(struct picoui_screen));
}

int picoui_screen_load(struct picoui_screen *screen)
{
    if (screen == 0) {
        return -1;
    }

    screen->loaded = 1;
    picoui_runtime_state()->active_screen = screen;
    return 0;
}

int picoui_screen_set_root_window(struct picoui_screen *screen, struct picoui_window *root_window)
{
    if (screen == 0 || root_window == 0) {
        return -1;
    }

    screen->root_window = root_window;
    return 0;
}

struct picoui_window *picoui_screen_get_root_window(const struct picoui_screen *screen)
{
    if (screen == 0) {
        return 0;
    }

    return screen->root_window;
}
