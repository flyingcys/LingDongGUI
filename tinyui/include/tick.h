#ifndef TINYUI_TICK_H
#define TINYUI_TICK_H

struct tinyui_app;

typedef unsigned int (*tinyui_tick_get_cb_t)(void *user_data);

int tinyui_tick_set_source(struct tinyui_app *app,
                           tinyui_tick_get_cb_t callback,
                           void *user_data);
unsigned int tinyui_tick_get(struct tinyui_app *app);

#endif
