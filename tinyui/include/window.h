#ifndef TINYUI_WINDOW_H
#define TINYUI_WINDOW_H

struct tinyui_app;
struct tinyui_image_source;
struct tinyui_window;

enum tinyui_window_layout_type {
    TINYUI_WINDOW_LAYOUT_NONE,
    TINYUI_WINDOW_LAYOUT_FLEX,
    TINYUI_WINDOW_LAYOUT_GRID,
};

struct tinyui_window_props {
    const char *id;
    const char *style_class;
    void *user_data;
    unsigned int bg_color;
    unsigned int text_color;
    unsigned int border_color;
    int radius;
    int padding;
    struct tinyui_image_source *background_source;
    /* Sentinel defaults: -1 = unset (use backend default padding on that edge) */
    int padding_left;
    int padding_top;
    int padding_right;
    int padding_bottom;
};

struct tinyui_window *tinyui_window_create(struct tinyui_app *app, const char *id);

struct tinyui_window *tinyui_window_create_child(struct tinyui_window *parent, const char *id);

struct tinyui_window *tinyui_window_create_with_props(struct tinyui_app *app,
                                                      const struct tinyui_window_props *props);

int tinyui_window_set_background_source(struct tinyui_window *window,
                                        struct tinyui_image_source *source);

int tinyui_window_set_background_offset(struct tinyui_window *window, int offset_x, int offset_y);

int tinyui_window_get_background_offset(struct tinyui_window *window,
                                        int *offset_x,
                                        int *offset_y);

int tinyui_window_set_color(struct tinyui_window *window, unsigned int rgb);

int tinyui_window_get_color(struct tinyui_window *window, unsigned int *rgb);

int tinyui_window_set_layout_type(struct tinyui_window *window,
                                  enum tinyui_window_layout_type type);

int tinyui_window_set_padding(struct tinyui_window *window,
                              int left,
                              int top,
                              int right,
                              int bottom);

int tinyui_window_set_grid_padding(struct tinyui_window *window,
                                   int left,
                                   int top,
                                   int right,
                                   int bottom);

int tinyui_window_set_gap(struct tinyui_window *window, int gap);

int tinyui_window_get_padding_group(struct tinyui_window *window,
                                    int *left,
                                    int *top,
                                    int *right,
                                    int *bottom);

int tinyui_window_get_layout_type(struct tinyui_window *window,
                                  enum tinyui_window_layout_type *type);

int tinyui_window_get_gap(struct tinyui_window *window);

#endif
