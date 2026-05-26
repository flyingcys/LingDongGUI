#include "internal.h"
#include "ldWindow.h"

#include <stdlib.h>

#define PICOUI_RUNTIME_ROOT_WIDTH 480
#define PICOUI_RUNTIME_ROOT_HEIGHT 320

static struct picoui_backend_app_state *picoui_backend_window_get_app_state(struct picoui_app *app)
{
    if (app == NULL || app->backend_app == NULL) {
        return NULL;
    }
    return (struct picoui_backend_app_state *)app->backend_app;
}

void *picoui_backend_create_window(struct picoui_app *app, const char *id)
{
    struct picoui_backend_widget *widget;
    struct picoui_backend_app_state *app_state;
    ldWindow_t *ld_root;

    if (app == 0 || id == 0) {
        return 0;
    }

    app_state = picoui_backend_window_get_app_state(app);
    if (app_state == NULL || app_state->ld_scene == NULL) {
        return 0;
    }

    widget = calloc(1, sizeof(*widget));
    if (widget == 0) {
        return 0;
    }

    ld_root = ldWindow_init(app_state->ld_scene,
                            NULL,
                            0,
                            0,
                            0,
                            0,
                            PICOUI_RUNTIME_ROOT_WIDTH,
                            PICOUI_RUNTIME_ROOT_HEIGHT);
    if (ld_root == NULL) {
        free(widget);
        return 0;
    }

    widget->id = id;
    widget->owner = app;
    widget->kind = PICOUI_BACKEND_WIDGET_WINDOW;
    widget->root = widget;
    widget->theme = app->theme;
    widget->ld_widget = ld_root;
    widget->ld_name_id = 0;
    return widget;
}
