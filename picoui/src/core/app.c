#include "internal.h"
#include "picoui/app.h"

#include <stdlib.h>

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
    if (app == NULL || window == NULL) {
        return -1;
    }

    app->root_window = window;
    return picoui_backend_app_run(app, window);
}

void picoui_app_destroy(struct picoui_app *app)
{
    if (app == NULL) {
        return;
    }

    picoui_backend_app_shutdown(app);
    free(app);
}
