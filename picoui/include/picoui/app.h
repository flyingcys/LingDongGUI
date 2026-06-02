#ifndef PICOUI_APP_H
#define PICOUI_APP_H

struct picoui_app;
struct picoui_background;
struct picoui_window;

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
void picoui_app_destroy(struct picoui_app *app);

#endif
