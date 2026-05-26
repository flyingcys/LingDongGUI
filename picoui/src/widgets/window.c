#include "internal.h"
#include "picoui/window.h"

#include <stdlib.h>

struct picoui_window *picoui_window_create(struct picoui_app *app, const char *id)
{
    struct picoui_window *window;
    void *backend_widget;

    if (app == 0 || id == 0) {
        return 0;
    }

    backend_widget = picoui_backend_create_window(app, id);
    if (backend_widget == 0) {
        return 0;
    }

    window = calloc(1, sizeof(*window));
    if (window == 0) {
        free(backend_widget);
        return 0;
    }

    window->id = id;
    window->widget.backend_widget = backend_widget;
    window->widget.visible = 1;
    window->widget.enabled = 1;
    return window;
}
