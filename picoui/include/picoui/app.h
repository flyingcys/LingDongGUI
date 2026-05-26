#ifndef PICOUI_APP_H
#define PICOUI_APP_H

struct picoui_app;
struct picoui_window;

struct picoui_app *picoui_app_create(void);
int picoui_app_run(struct picoui_app *app, struct picoui_window *window);
void picoui_app_destroy(struct picoui_app *app);

#endif
