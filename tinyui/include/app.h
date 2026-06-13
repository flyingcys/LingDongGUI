#ifndef TINYUI_APP_H
#define TINYUI_APP_H

/*
 * INTERNAL / NON-CANONICAL -- DO NOT USE FOR NEW CODE.
 *
 * This header exposes the old tinyui_app_* startup path, which has been
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

struct tinyui_app;
struct tinyui_app_timer;
struct tinyui_background;
struct tinyui_window;

typedef void (*tinyui_app_timer_cb_t)(struct tinyui_app *app,
                                      struct tinyui_app_timer *timer,
                                      void *user_data);

struct tinyui_app *tinyui_app_create(void);
int tinyui_app_run(struct tinyui_app *app, struct tinyui_window *window);
int tinyui_app_run_background(struct tinyui_app *app, struct tinyui_background *background);
int tinyui_app_set_window(struct tinyui_app *app, struct tinyui_window *window);
int tinyui_app_set_background(struct tinyui_app *app, struct tinyui_background *background);
int tinyui_app_switch_window(struct tinyui_app *app,
                             struct tinyui_window *window,
                             int mode,
                             unsigned int duration_ms);
int tinyui_app_switch_background(struct tinyui_app *app,
                                 struct tinyui_background *background,
                                 int mode,
                                 unsigned int duration_ms);
struct tinyui_app_timer *tinyui_app_timer_create(struct tinyui_app *app);
int tinyui_app_timer_start(struct tinyui_app_timer *timer,
                           unsigned int interval_ms,
                           int repeat,
                           tinyui_app_timer_cb_t callback,
                           void *user_data);
int tinyui_app_timer_stop(struct tinyui_app_timer *timer);
int tinyui_app_timer_is_running(const struct tinyui_app_timer *timer);
void tinyui_app_timer_destroy(struct tinyui_app_timer *timer);
void tinyui_app_destroy(struct tinyui_app *app);

#endif
