#include "internal.h"

#include <stdlib.h>

int picoui_backend_apply_theme(struct picoui_app *app, struct picoui_theme *theme)
{
    if (app == 0 || theme == 0) {
        return -1;
    }

    app->theme = theme;
    if (app->backend_app == 0) {
        app->backend_app = calloc(1, sizeof(struct picoui_backend_app_state));
        if (app->backend_app == 0) {
            return -1;
        }
    }

    ((struct picoui_backend_app_state *)app->backend_app)->theme = theme;
    return 0;
}
