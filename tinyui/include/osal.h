#ifndef TINYUI_OSAL_H
#define TINYUI_OSAL_H

struct picoui_app;

typedef void (*picoui_os_lock_cb_t)(void *user_data);
typedef void (*picoui_os_delay_cb_t)(unsigned int ms, void *user_data);

int picoui_os_set_lock_callbacks(struct picoui_app *app,
                                 picoui_os_lock_cb_t enter,
                                 picoui_os_lock_cb_t leave,
                                 void *user_data);
void picoui_os_enter(struct picoui_app *app);
void picoui_os_leave(struct picoui_app *app);
int picoui_os_set_delay_callback(struct picoui_app *app,
                                 picoui_os_delay_cb_t delay,
                                 void *user_data);
void picoui_os_delay(struct picoui_app *app, unsigned int ms);

#endif
