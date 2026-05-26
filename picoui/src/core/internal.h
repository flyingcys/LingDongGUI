#ifndef PICOUI_INTERNAL_H
#define PICOUI_INTERNAL_H

#include "backend.h"
#include "picoui/theme.h"

#define PICOUI_LAYOUT_MAX_TRACKS 16

typedef struct arm_2d_tile_t arm_2d_tile_t;

struct picoui_font;

struct picoui_widget {
    void *backend_widget;
    int x;
    int y;
    int width;
    int height;
    const char *text;
    const char *style_class;
    void *user_data;
    unsigned int bg_color;
    unsigned int text_color;
    unsigned int border_color;
    int radius;
    int padding;
    const struct picoui_font *font;
    int visible;
    int enabled;
    int flex_grow;
    int flex_new_track;
    int ignore_layout;
    int grid_col;
    int grid_row;
    int grid_col_span;
    int grid_row_span;
    enum picoui_align grid_x_align;
    enum picoui_align grid_y_align;
};

struct picoui_app {
    void *backend_app;
    struct picoui_theme *theme;
    struct picoui_window *root_window;
};

struct picoui_theme {
    unsigned int colors[PICOUI_COLOR_COUNT];
    int metrics[PICOUI_METRIC_COUNT];
};

struct picoui_window {
    struct picoui_widget widget;
    const char *id;
    enum picoui_flex_flow flex_flow;
    enum picoui_align flex_main_align;
    enum picoui_align flex_cross_align;
    enum picoui_align flex_track_align;
    int flex_item_gap;
    int flex_track_gap;
    int grid_cols[PICOUI_LAYOUT_MAX_TRACKS];
    int grid_rows[PICOUI_LAYOUT_MAX_TRACKS];
    int grid_col_count;
    int grid_row_count;
    int grid_row_gap;
    int grid_col_gap;
    enum picoui_align grid_col_align;
    enum picoui_align grid_row_align;
};

struct picoui_label {
    struct picoui_widget widget;
    const char *id;
};

struct picoui_button {
    struct picoui_widget widget;
    const char *id;
    picoui_event_cb on_clicked;
    void *user_data;
    picoui_event_cb on_pressed;
    void *on_pressed_user_data;
    picoui_event_cb on_released;
    void *on_released_user_data;
};

struct picoui_checkbox {
    struct picoui_widget widget;
    const char *id;
    int checked;
    picoui_value_changed_cb cb;
    void *user_data;
};

struct picoui_switch {
    struct picoui_widget widget;
    const char *id;
    int checked;
    picoui_value_changed_cb cb;
    void *user_data;
};

struct picoui_slider {
    struct picoui_widget widget;
    const char *id;
    int value;
    int min_value;
    int max_value;
    picoui_value_changed_cb cb;
    void *user_data;
};

struct picoui_text {
    struct picoui_widget widget;
    const char *id;
};

struct picoui_image_source {
    arm_2d_tile_t *img_tile;
    arm_2d_tile_t *mask_tile;
};

struct picoui_image {
    struct picoui_widget widget;
    const char *id;
    struct picoui_image_source *source;
};

#endif
