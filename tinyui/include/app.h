#ifndef TINYUI_APP_H
#define TINYUI_APP_H

/*
 * INTERNAL / NON-CANONICAL -- DO NOT USE FOR NEW CODE.
 *
 * This header exposes the old picoui_app_* startup path, which has been
 * superseded by the canonical tinyui_init / tinyui_screen_create /
 * tinyui_screen_load / tinyui_timer_handler chain declared in runtime.h.
 *
 * New user code must #include "runtime.h" and use the tinyui_* API.
 * This file is retained only for internal lifecycle helpers (timer pump,
 * app/set_window) and for existing demo source files that have not yet
 * been migrated to the canonical path.  All such legacy consumers will
 * be migrated in a future stage.
 *
 * Removal target: after all demo/* and internal callers are migrated.
 */

struct picoui_app;
struct picoui_app_timer;
struct picoui_background;
struct picoui_window;

typedef void (*picoui_app_timer_cb_t)(struct picoui_app *app,
                                      struct picoui_app_timer *timer,
                                      void *user_data);

struct picoui_app *picoui_app_create(void);
int picoui_app_run(struct picoui_app *app, struct picoui_window *window);
int picoui_app_run_background(struct picoui_app *app, struct picoui_background *background);
int picoui_app_set_window(struct picoui_app *app, struct picoui_window *window);
int picoui_app_set_background(struct picoui_app *app, struct picoui_background *background);
int picoui_app_switch_window(struct picoui_app *app,
                             struct picoui_window *window,
                             int mode,
                             unsigned int duration_ms);
int picoui_app_switch_background(struct picoui_app *app,
                                 struct picoui_background *background,
                                 int mode,
                                 unsigned int duration_ms);
struct picoui_app_timer *picoui_app_timer_create(struct picoui_app *app);
int picoui_app_timer_start(struct picoui_app_timer *timer,
                           unsigned int interval_ms,
                           int repeat,
                           picoui_app_timer_cb_t callback,
                           void *user_data);
int picoui_app_timer_stop(struct picoui_app_timer *timer);
int picoui_app_timer_is_running(const struct picoui_app_timer *timer);
void picoui_app_timer_destroy(struct picoui_app_timer *timer);
void picoui_app_destroy(struct picoui_app *app);

#endif
