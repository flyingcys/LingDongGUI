#include "internal.h"
#include "picoui/osal.h"

#include <stddef.h>

int picoui_os_set_lock_callbacks(struct picoui_app *app,
                                 picoui_os_lock_cb_t enter,
                                 picoui_os_lock_cb_t leave,
                                 void *user_data)
{
    if (app == NULL) {
        return -1;
    }

    app->os_port.enter = enter;
    app->os_port.leave = leave;
    app->os_port.lock_user_data = user_data;
    return 0;
}

void picoui_os_enter(struct picoui_app *app)
{
    if (app != NULL && app->os_port.enter != NULL) {
        app->os_port.enter(app->os_port.lock_user_data);
    }
}

void picoui_os_leave(struct picoui_app *app)
{
    if (app != NULL && app->os_port.leave != NULL) {
        app->os_port.leave(app->os_port.lock_user_data);
    }
}

int picoui_os_set_delay_callback(struct picoui_app *app,
                                 picoui_os_delay_cb_t delay,
                                 void *user_data)
{
    if (app == NULL) {
        return -1;
    }

    app->os_port.delay = delay;
    app->os_port.delay_user_data = user_data;
    return 0;
}

void picoui_os_delay(struct picoui_app *app, unsigned int ms)
{
    if (app != NULL && app->os_port.delay != NULL) {
        app->os_port.delay(ms, app->os_port.delay_user_data);
    }
}
