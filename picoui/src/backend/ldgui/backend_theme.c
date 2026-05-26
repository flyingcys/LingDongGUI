#include "backend.h"

int picoui_backend_apply_theme(struct picoui_app *app, struct picoui_theme *theme)
{
    if (app == 0 || theme == 0) {
        return -1;
    }

    return 0;
}
