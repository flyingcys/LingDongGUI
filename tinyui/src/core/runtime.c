#include "internal.h"
#include "runtime_bridge.h"
#include "runtime.h"
#include "window.h"

#include <stdlib.h>

static struct picoui_app *g_tinyui_runtime_app;

int picoui_init(void)
{
    if (g_tinyui_runtime_app != 0) {
        return 0;
    }

    g_tinyui_runtime_app = picoui_app_create();
    return g_tinyui_runtime_app != 0 ? 0 : -1;
}

void picoui_deinit(void)
{
    if (g_tinyui_runtime_app == 0) {
        return;
    }

    picoui_app_destroy(g_tinyui_runtime_app);
    g_tinyui_runtime_app = 0;
}

struct picoui_window *picoui_screen_create(void)
{
    if (g_tinyui_runtime_app == 0 && picoui_init() != 0) {
        return 0;
    }

    return picoui_window_create(g_tinyui_runtime_app, "root");
}

int picoui_screen_load(struct picoui_window *screen)
{
    if (g_tinyui_runtime_app == 0 || screen == 0) {
        return -1;
    }

    return picoui_app_set_window(g_tinyui_runtime_app, screen);
}

void picoui_timer_handler(void)
{
    int step;

    if (g_tinyui_runtime_app == 0) {
        return;
    }

    step = tinyui_runtime_bridge_step_app(g_tinyui_runtime_app);
    if (step < 0) {
        picoui_deinit();
        exit(1);
    }
    if (step > 0) {
        picoui_deinit();
        exit(0);
    }
}
