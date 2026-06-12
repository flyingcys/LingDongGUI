#ifndef TINYUI_APP_H
#define TINYUI_APP_H

/*
 * Compatibility API retained during TinyUI transition.
 * New user-facing startup path should use runtime.h.
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
