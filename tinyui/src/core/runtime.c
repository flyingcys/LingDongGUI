#include "internal.h"
#include "core/runtime.h"
#include "core/timer.h"
#include "integration/input.h"
#include "runtime_bridge.h"
#include "widgets/background.h"
#include "widgets/window.h"

#include <stdlib.h>

struct tinyui_obj;

static struct tinyui_app *g_tinyui_runtime_app;
static tinyui_result_t g_tinyui_last_result = TINYUI_OK;

extern void tinyui_internal_theme_reset(void);

void tinyui_runtime_set_last_result(tinyui_result_t result)
{
    g_tinyui_last_result = result;
}

tinyui_result_t tinyui_last_result(void)
{
    return g_tinyui_last_result;
}

const char *tinyui_last_error_message(void)
{
    switch (g_tinyui_last_result) {
    case TINYUI_OK:
        return "TINYUI_OK";
    case TINYUI_ERROR_INVALID_ARG:
        return "TINYUI_ERROR_INVALID_ARG";
    case TINYUI_ERROR_INVALID_OBJECT:
        return "TINYUI_ERROR_INVALID_OBJECT";
    case TINYUI_ERROR_INVALID_STATE:
        return "TINYUI_ERROR_INVALID_STATE";
    case TINYUI_ERROR_NOT_SUPPORTED:
        return "TINYUI_ERROR_NOT_SUPPORTED";
    case TINYUI_ERROR_OUT_OF_RANGE:
        return "TINYUI_ERROR_OUT_OF_RANGE";
    case TINYUI_ERROR_NO_MEMORY:
        return "TINYUI_ERROR_NO_MEMORY";
    case TINYUI_ERROR_CAPACITY:
        return "TINYUI_ERROR_CAPACITY";
    case TINYUI_ERROR_BACKEND:
        return "TINYUI_ERROR_BACKEND";
    default:
        return "TINYUI_ERROR_UNKNOWN";
    }
}

tinyui_result_t tinyui_init(void)
{
    if (g_tinyui_runtime_app != 0) {
        tinyui_runtime_set_last_result(TINYUI_OK);
        return TINYUI_OK;
    }

    g_tinyui_runtime_app = tinyui_app_create();
    tinyui_runtime_set_last_result(g_tinyui_runtime_app != 0 ? TINYUI_OK
                                                              : TINYUI_ERROR_NO_MEMORY);
    return g_tinyui_last_result;
}

void tinyui_deinit(void)
{
    tinyui_internal_theme_reset();
    if (g_tinyui_runtime_app == 0) {
        return;
    }

    tinyui_app_destroy(g_tinyui_runtime_app);
    g_tinyui_runtime_app = 0;
    tinyui_runtime_set_last_result(TINYUI_OK);
}

struct tinyui_app *tinyui_app_current(void)
{
    return g_tinyui_runtime_app;
}

tinyui_obj_t *tinyui_screen_create(void)
{
    if (g_tinyui_runtime_app == 0 && tinyui_init() != TINYUI_OK) {
        return 0;
    }

    tinyui_runtime_bridge_begin_screen_create(g_tinyui_runtime_app);
    return (struct tinyui_obj *)tinyui_window_create(g_tinyui_runtime_app, "root");
}

tinyui_obj_t *tinyui_screen_create_with_props(const tinyui_window_props_t *props)
{
    if (props == 0) {
        return 0;
    }

    if (g_tinyui_runtime_app == 0 && tinyui_init() != TINYUI_OK) {
        return 0;
    }

    return (tinyui_obj_t *)tinyui_window_create_with_props(
        g_tinyui_runtime_app,
        (const struct tinyui_window_props *)props);
}

tinyui_obj_t *tinyui_background_create(void)
{
    if (g_tinyui_runtime_app == 0 && tinyui_init() != TINYUI_OK) {
        return 0;
    }

    return (tinyui_obj_t *)tinyui_legacy_background_create(
        g_tinyui_runtime_app,
        "background");
}

tinyui_obj_t *tinyui_background_create_with_props(const tinyui_background_props_t *props)
{
    (void)props;
    return 0;
}

tinyui_result_t tinyui_screen_load(tinyui_obj_t *screen,
                                   tinyui_screen_transition_t transition,
                                   uint32_t duration_ms)
{
    if (g_tinyui_runtime_app == 0) {
        return TINYUI_ERROR_INVALID_STATE;
    }
    if (screen == 0) {
        return TINYUI_ERROR_INVALID_OBJECT;
    }
    if (transition != TINYUI_SCREEN_TRANSITION_NONE) {
        (void)duration_ms;
        return TINYUI_ERROR_NOT_SUPPORTED;
    }
    if (!tinyui_runtime_bridge_window_is_owned_by(
            g_tinyui_runtime_app,
            (const struct tinyui_window *)screen)) {
        return TINYUI_ERROR_INVALID_OBJECT;
    }

    return tinyui_app_set_window(g_tinyui_runtime_app, (struct tinyui_window *)screen) == 0
               ? TINYUI_OK
               : TINYUI_ERROR_BACKEND;
}

tinyui_obj_t *tinyui_screen_active(void)
{
    if (g_tinyui_runtime_app == 0 || g_tinyui_runtime_app->root_window == 0) {
        return 0;
    }

    return (tinyui_obj_t *)&g_tinyui_runtime_app->root_window->widget;
}

tinyui_result_t tinyui_process(uint32_t *next_ms)
{
    int step;

    if (next_ms == 0) {
        tinyui_runtime_set_last_result(TINYUI_ERROR_INVALID_ARG);
        return TINYUI_ERROR_INVALID_ARG;
    }
    if (g_tinyui_runtime_app == 0) {
        tinyui_runtime_set_last_result(TINYUI_ERROR_INVALID_STATE);
        return TINYUI_ERROR_INVALID_STATE;
    }

    *next_ms = UINT32_MAX;
    step = tinyui_runtime_bridge_step_app(g_tinyui_runtime_app);
    tinyui_runtime_set_last_result(step < 0 ? TINYUI_ERROR_BACKEND : TINYUI_OK);
    return g_tinyui_last_result;
}

tinyui_timer_t *tinyui_timer_create(uint32_t interval_ms,
                                    bool repeat,
                                    tinyui_timer_cb_t cb,
                                    void *user_data)
{
    (void)interval_ms;
    (void)repeat;
    (void)cb;
    (void)user_data;
    tinyui_runtime_set_last_result(TINYUI_ERROR_NOT_SUPPORTED);
    return NULL;
}

tinyui_result_t tinyui_timer_start(tinyui_timer_t *timer)
{
    (void)timer;
    tinyui_runtime_set_last_result(TINYUI_ERROR_NOT_SUPPORTED);
    return TINYUI_ERROR_NOT_SUPPORTED;
}

tinyui_result_t tinyui_timer_stop(tinyui_timer_t *timer)
{
    (void)timer;
    tinyui_runtime_set_last_result(TINYUI_ERROR_NOT_SUPPORTED);
    return TINYUI_ERROR_NOT_SUPPORTED;
}

tinyui_result_t tinyui_timer_set_interval(tinyui_timer_t *timer, uint32_t interval_ms)
{
    (void)timer;
    (void)interval_ms;
    tinyui_runtime_set_last_result(TINYUI_ERROR_NOT_SUPPORTED);
    return TINYUI_ERROR_NOT_SUPPORTED;
}

void tinyui_timer_delete(tinyui_timer_t *timer)
{
    (void)timer;
    tinyui_runtime_set_last_result(TINYUI_ERROR_NOT_SUPPORTED);
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
