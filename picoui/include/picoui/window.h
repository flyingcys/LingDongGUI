#ifndef PICOUI_WINDOW_H
#define PICOUI_WINDOW_H

struct picoui_app;
struct picoui_image_source;
struct picoui_window;

enum picoui_window_layout_type {
    PICOUI_WINDOW_LAYOUT_NONE,
    PICOUI_WINDOW_LAYOUT_FLEX,
    PICOUI_WINDOW_LAYOUT_GRID,
};

struct picoui_window_props {
    const char *id;
    const char *style_class;
    void *user_data;
    unsigned int bg_color;
    unsigned int text_color;
    unsigned int border_color;
    int radius;
    int padding;
    struct picoui_image_source *background_source;
    int has_padding_group;
    int padding_left;
    int padding_top;
    int padding_right;
    int padding_bottom;
};

struct picoui_window *picoui_window_create(struct picoui_app *app, const char *id);
struct picoui_window *picoui_window_create_with_props(struct picoui_app *app,
                                                      const struct picoui_window_props *props);
int picoui_window_set_background_source(struct picoui_window *window,
                                        struct picoui_image_source *source);
int picoui_window_set_color(struct picoui_window *window, unsigned int rgb);
int picoui_window_get_color(struct picoui_window *window, unsigned int *rgb);
int picoui_window_set_padding_group(struct picoui_window *window,
                                    int left,
                                    int top,
                                    int right,
                                    int bottom);
int picoui_window_set_layout_type(struct picoui_window *window,
                                  enum picoui_window_layout_type type);
int picoui_window_set_padding(struct picoui_window *window,
                              int left,
                              int top,
                              int right,
                              int bottom);
int picoui_window_set_grid_padding(struct picoui_window *window,
                                   int left,
                                   int top,
                                   int right,
                                   int bottom);
int picoui_window_set_gap(struct picoui_window *window, int gap);
int picoui_window_get_padding_left(struct picoui_window *window);
int picoui_window_get_padding_top(struct picoui_window *window);
int picoui_window_get_padding_right(struct picoui_window *window);
int picoui_window_get_padding_bottom(struct picoui_window *window);

#endif
