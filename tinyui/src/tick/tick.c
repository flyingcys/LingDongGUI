#include "internal.h"
#include "tick.h"

#include <stddef.h>

int picoui_tick_set_source(struct picoui_app *app,
                           picoui_tick_get_cb_t callback,
                           void *user_data)
{
    if (app == NULL) {
        return -1;
    }

    app->tick_port.callback = callback;
    app->tick_port.user_data = user_data;
    return 0;
}

unsigned int picoui_tick_get(struct picoui_app *app)
{
    if (app == NULL || app->tick_port.callback == NULL) {
        return 0;
    }

    return app->tick_port.callback(app->tick_port.user_data);
}
