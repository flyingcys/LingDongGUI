#include "internal.h"
#include "osal/osal.h"

#include <stddef.h>

int tinyui_os_set_lock_callbacks(struct tinyui_app *app,
                                 tinyui_os_lock_cb_t enter,
                                 tinyui_os_lock_cb_t leave,
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

void tinyui_os_enter(struct tinyui_app *app)
{
    if (app != NULL && app->os_port.enter != NULL) {
        app->os_port.enter(app->os_port.lock_user_data);
    }
}

void tinyui_os_leave(struct tinyui_app *app)
{
    if (app != NULL && app->os_port.leave != NULL) {
        app->os_port.leave(app->os_port.lock_user_data);
    }
}

int tinyui_os_set_delay_callback(struct tinyui_app *app,
                                 tinyui_os_delay_cb_t delay,
                                 void *user_data)
{
    if (app == NULL) {
        return -1;
    }

    app->os_port.delay = delay;
    app->os_port.delay_user_data = user_data;
    return 0;
}

void tinyui_os_delay(struct tinyui_app *app, unsigned int ms)
{
    if (app != NULL && app->os_port.delay != NULL) {
        app->os_port.delay(ms, app->os_port.delay_user_data);
    }
}
