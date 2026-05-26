#ifndef PICOUI_WINDOW_H
#define PICOUI_WINDOW_H

struct picoui_app;
struct picoui_window;

struct picoui_window *picoui_window_create(struct picoui_app *app, const char *id);

#endif
