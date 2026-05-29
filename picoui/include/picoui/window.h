#ifndef PICOUI_WINDOW_H
#define PICOUI_WINDOW_H

struct picoui_app;
struct picoui_window;

struct picoui_window_props {
    const char *id;
    const char *style_class;
    void *user_data;
    unsigned int bg_color;
    unsigned int text_color;
    unsigned int border_color;
    int radius;
    int padding;
};

struct picoui_window *picoui_window_create(struct picoui_app *app, const char *id);
struct picoui_window *picoui_window_create_with_props(struct picoui_app *app,
                                                      const struct picoui_window_props *props);

#endif
