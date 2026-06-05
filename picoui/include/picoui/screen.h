#ifndef PICOUI_SCREEN_H
#define PICOUI_SCREEN_H

struct picoui_window;
struct picoui_screen;

struct picoui_screen *picoui_screen_active(void);
struct picoui_screen *picoui_screen_create(void);
int picoui_screen_load(struct picoui_screen *screen);
int picoui_screen_set_root_window(struct picoui_screen *screen, struct picoui_window *root_window);
struct picoui_window *picoui_screen_get_root_window(const struct picoui_screen *screen);

#endif
