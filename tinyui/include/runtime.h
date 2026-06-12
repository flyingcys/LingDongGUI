#ifndef TINYUI_RUNTIME_H
#define TINYUI_RUNTIME_H

#include "widget.h"

struct picoui_window;

int picoui_init(void);
void picoui_deinit(void);
struct picoui_window *picoui_screen_create(void);
int picoui_screen_load(struct picoui_window *screen);
void picoui_timer_handler(void);

#define tinyui_init picoui_init
#define tinyui_deinit picoui_deinit
#define tinyui_timer_handler picoui_timer_handler

static inline tinyui_obj_t *tinyui_screen_create(void)
{
    return (tinyui_obj_t *)picoui_screen_create();
}

static inline int tinyui_screen_load(tinyui_obj_t *screen)
{
    return picoui_screen_load((struct picoui_window *)screen);
}

#endif
