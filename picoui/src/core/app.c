#include "internal.h"
#include "picoui/app.h"
#include "picoui/background.h"
#include "../../../src/misc/xBtnAction.h"

#include <stdlib.h>

static int picoui_app_window_is_owned_by(const struct picoui_app *app,
                                         const struct picoui_window *window)
{
    const struct picoui_backend_widget *backend_widget;

    if (app == NULL || window == NULL || window->widget.backend_widget == NULL) {
        return 0;
    }

    backend_widget = (const struct picoui_backend_widget *)window->widget.backend_widget;
    return backend_widget->owner == app;
}

struct picoui_app *picoui_app_create(void)
{
    struct picoui_app *app = calloc(1, sizeof(struct picoui_app));
    if (app == NULL) {
        return NULL;
    }

    if (picoui_backend_app_init(app) != 0) {
        free(app);
        return NULL;
    }

    return app;
}

int picoui_app_run(struct picoui_app *app, struct picoui_window *window)
{
    if (app == NULL || !picoui_app_window_is_owned_by(app, window)) {
        return -1;
    }

    app->root_window = window;
    return picoui_backend_app_run(app, window);
}

int picoui_app_run_background(struct picoui_app *app, struct picoui_background *background)
{
    return picoui_app_run(app, (struct picoui_window *)background);
}

int picoui_app_set_window(struct picoui_app *app, struct picoui_window *window)
{
    if (app == NULL || !picoui_app_window_is_owned_by(app, window)) {
        return -1;
    }

    app->root_window = window;
    app->focus_owner = 0;
    app->editing_owner = 0;
    if (app->backend_app != NULL) {
        struct picoui_backend_app_state *app_state =
            (struct picoui_backend_app_state *)app->backend_app;
        app_state->last_window_switch_mode = 0;
        app_state->last_window_switch_duration_ms = 0;
    }
    return 0;
}

int picoui_app_set_background(struct picoui_app *app, struct picoui_background *background)
{
    return picoui_app_set_window(app, (struct picoui_window *)background);
}

int picoui_app_switch_window(struct picoui_app *app,
                             struct picoui_window *window,
                             int mode,
                             unsigned int duration_ms)
{
    if (picoui_app_set_window(app, window) != 0) {
        return -1;
    }

    if (app->backend_app != NULL) {
        struct picoui_backend_app_state *app_state =
            (struct picoui_backend_app_state *)app->backend_app;
        app_state->last_window_switch_mode = mode;
        app_state->last_window_switch_duration_ms = duration_ms;
    }
    return 0;
}

int picoui_app_switch_background(struct picoui_app *app,
                                 struct picoui_background *background,
                                 int mode,
                                 unsigned int duration_ms)
{
    return picoui_app_switch_window(app, (struct picoui_window *)background, mode, duration_ms);
}

void picoui_app_destroy(struct picoui_app *app)
{
    if (app == NULL) {
        return;
    }

    picoui_backend_app_shutdown(app);
    xBtnDestroy();
    free(app);
}
