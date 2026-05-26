#include "internal.h"

#include <stdlib.h>

void *picoui_backend_create_window(struct picoui_app *app, const char *id)
{
    struct picoui_backend_widget *widget;

    if (app == 0 || id == 0) {
        return 0;
    }

    widget = calloc(1, sizeof(*widget));
    if (widget == 0) {
        return 0;
    }

    widget->id = id;
    widget->theme = app->theme;
    return widget;
}
