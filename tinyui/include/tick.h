#ifndef TINYUI_TICK_H
#define TINYUI_TICK_H

struct picoui_app;

typedef unsigned int (*picoui_tick_get_cb_t)(void *user_data);

int picoui_tick_set_source(struct picoui_app *app,
                           picoui_tick_get_cb_t callback,
                           void *user_data);
unsigned int picoui_tick_get(struct picoui_app *app);

#endif
