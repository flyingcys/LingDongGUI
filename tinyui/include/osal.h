#ifndef TINYUI_OSAL_H
#define TINYUI_OSAL_H

struct tinyui_app;

typedef void (*tinyui_os_lock_cb_t)(void *user_data);
typedef void (*tinyui_os_delay_cb_t)(unsigned int ms, void *user_data);

int tinyui_os_set_lock_callbacks(struct tinyui_app *app,
                                 tinyui_os_lock_cb_t enter,
                                 tinyui_os_lock_cb_t leave,
                                 void *user_data);
void tinyui_os_enter(struct tinyui_app *app);
void tinyui_os_leave(struct tinyui_app *app);
int tinyui_os_set_delay_callback(struct tinyui_app *app,
                                 tinyui_os_delay_cb_t delay,
                                 void *user_data);
void tinyui_os_delay(struct tinyui_app *app, unsigned int ms);

#endif
