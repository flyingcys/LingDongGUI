#include "internal.h"
#include "runtime_bridge.h"
#include "runtime.h"
#include "window.h"

#include <stdlib.h>

static struct tinyui_app *g_tinyui_runtime_app;

int tinyui_init(void)
{
    if (g_tinyui_runtime_app != 0) {
        return 0;
    }

    g_tinyui_runtime_app = tinyui_app_create();
    return g_tinyui_runtime_app != 0 ? 0 : -1;
}

void tinyui_deinit(void)
{
    if (g_tinyui_runtime_app == 0) {
        return;
    }

    tinyui_app_destroy(g_tinyui_runtime_app);
    g_tinyui_runtime_app = 0;
}

tinyui_obj_t *tinyui_screen_create(void)
{
    if (g_tinyui_runtime_app == 0 && tinyui_init() != 0) {
        return 0;
    }

    tinyui_runtime_bridge_begin_screen_create(g_tinyui_runtime_app);
    return (tinyui_obj_t *)tinyui_window_create(g_tinyui_runtime_app, "root");
}

int tinyui_screen_load(tinyui_obj_t *screen)
{
    if (g_tinyui_runtime_app == 0 || screen == 0) {
        return -1;
    }

    return tinyui_app_set_window(g_tinyui_runtime_app, (struct tinyui_window *)screen);
}

int tinyui_timer_handler(void)
{
    int step;

    if (g_tinyui_runtime_app == 0) {
        return -1;
    }

    step = tinyui_runtime_bridge_step_app(g_tinyui_runtime_app);
    if (step < 0) {
        return -1;
    }
    if (step > 0) {
        return 1;
    }
    return 0;
}
