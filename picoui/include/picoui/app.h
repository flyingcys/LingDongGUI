#ifndef PICOUI_APP_H
#define PICOUI_APP_H

struct picoui_app;
struct picoui_window;

struct picoui_app *picoui_app_create(void);
void picoui_app_destroy(struct picoui_app *app);

#endif
