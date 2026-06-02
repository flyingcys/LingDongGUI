#ifndef PICOUI_WIDGET_H
#define PICOUI_WIDGET_H

#include "picoui/layout.h"
#include "picoui/native.h"
#include "picoui/theme.h"

struct picoui_widget;
struct picoui_app;
struct picoui_font;

typedef void (*picoui_value_changed_cb)(struct picoui_widget *widget,
                                        int value,
                                        void *user_data);
typedef void (*picoui_event_cb)(struct picoui_widget *widget, void *user_data);

struct picoui_font {
    const char *family;
    int size;
};

struct picoui_point {
    int x;
    int y;
};

struct picoui_size {
    int width;
    int height;
};

struct picoui_rect {
    int x;
    int y;
    int width;
    int height;
};

enum picoui_widget_type {
    PICOUI_WIDGET_TYPE_UNKNOWN = 0,
    PICOUI_WIDGET_TYPE_BACKGROUND,
    PICOUI_WIDGET_TYPE_WINDOW,
    PICOUI_WIDGET_TYPE_BUTTON,
    PICOUI_WIDGET_TYPE_IMAGE,
    PICOUI_WIDGET_TYPE_TEXT,
    PICOUI_WIDGET_TYPE_LINE_EDIT,
    PICOUI_WIDGET_TYPE_GRAPH,
    PICOUI_WIDGET_TYPE_CHECKBOX,
    PICOUI_WIDGET_TYPE_SLIDER,
    PICOUI_WIDGET_TYPE_SWITCH,
    PICOUI_WIDGET_TYPE_PROGRESS_BAR,
    PICOUI_WIDGET_TYPE_GAUGE,
    PICOUI_WIDGET_TYPE_QRCODE,
    PICOUI_WIDGET_TYPE_DATE_TIME,
    PICOUI_WIDGET_TYPE_ICON_SLIDER,
    PICOUI_WIDGET_TYPE_COMBO_BOX,
    PICOUI_WIDGET_TYPE_ARC,
    PICOUI_WIDGET_TYPE_RADIAL_MENU,
    PICOUI_WIDGET_TYPE_SCROLL_SELECTER,
    PICOUI_WIDGET_TYPE_LABEL,
    PICOUI_WIDGET_TYPE_TABLE,
    PICOUI_WIDGET_TYPE_KEYBOARD,
    PICOUI_WIDGET_TYPE_ANIMATION,
    PICOUI_WIDGET_TYPE_LIST,
    PICOUI_WIDGET_TYPE_MESSAGE_BOX,
    PICOUI_WIDGET_TYPE_CALENDAR,
    PICOUI_WIDGET_TYPE_PROGRESS_WHEEL,
    PICOUI_WIDGET_TYPE_CLOCK,
};

int picoui_widget_set_pos(struct picoui_widget *widget, int x, int y);
int picoui_widget_set_size(struct picoui_widget *widget, int width, int height);
int picoui_widget_set_text(struct picoui_widget *widget, const char *text);
int picoui_widget_set_style_class(struct picoui_widget *widget, const char *style_class);
int picoui_widget_set_user_data(struct picoui_widget *widget, void *user_data);
int picoui_widget_set_bg_color(struct picoui_widget *widget, unsigned int rgb);
int picoui_widget_set_text_color(struct picoui_widget *widget, unsigned int rgb);
int picoui_widget_set_border_color(struct picoui_widget *widget, unsigned int rgb);
int picoui_widget_set_radius(struct picoui_widget *widget, int radius);
int picoui_widget_set_padding(struct picoui_widget *widget, int padding);
int picoui_widget_set_center(struct picoui_widget *widget);
int picoui_widget_set_visible(struct picoui_widget *widget, int visible);
int picoui_widget_set_opacity(struct picoui_widget *widget, int opacity);
int picoui_widget_set_selectable(struct picoui_widget *widget, int selectable);
int picoui_widget_set_selected(struct picoui_widget *widget, int selected);
int picoui_widget_set_corner(struct picoui_widget *widget, int corner);
int picoui_widget_set_enabled(struct picoui_widget *widget, int enabled);
int picoui_widget_set_flex_grow(struct picoui_widget *widget, int grow);
int picoui_widget_set_flex_new_track(struct picoui_widget *widget, int new_track);
int picoui_widget_set_flex_min_width(struct picoui_widget *widget, int min_width);
int picoui_widget_set_flex_min_height(struct picoui_widget *widget, int min_height);
int picoui_widget_set_flex_max_width(struct picoui_widget *widget, int max_width);
int picoui_widget_set_flex_max_height(struct picoui_widget *widget, int max_height);
int picoui_widget_set_ignore_layout(struct picoui_widget *widget, int ignore_layout);
int picoui_widget_set_grid_cell(struct picoui_widget *widget,
                                int col,
                                int row,
                                int col_span,
                                int row_span,
                                enum picoui_align x_align,
                                enum picoui_align y_align);
int picoui_widget_get_x(const struct picoui_widget *widget);
int picoui_widget_get_y(const struct picoui_widget *widget);
int picoui_widget_get_width(const struct picoui_widget *widget);
int picoui_widget_get_height(const struct picoui_widget *widget);
int picoui_widget_get_visible(const struct picoui_widget *widget);
int picoui_widget_get_opacity(const struct picoui_widget *widget);
int picoui_widget_get_selectable(const struct picoui_widget *widget);
int picoui_widget_get_selected(const struct picoui_widget *widget);
int picoui_widget_get_corner(const struct picoui_widget *widget);
struct picoui_widget *picoui_widget_get_parent(const struct picoui_widget *widget);
struct picoui_widget *picoui_widget_get_first_child(const struct picoui_widget *widget);
struct picoui_widget *picoui_widget_get_next_sibling(const struct picoui_widget *widget);
struct picoui_widget *picoui_widget_get_root(const struct picoui_widget *widget);
int picoui_widget_get_child_count(const struct picoui_widget *widget);
int picoui_widget_get_name_id(const struct picoui_widget *widget);
struct picoui_widget *picoui_widget_find_by_name_id(const struct picoui_widget *root, int name_id);
enum picoui_widget_type picoui_widget_get_type(const struct picoui_widget *widget);
struct picoui_point picoui_widget_get_absolute_pos(const struct picoui_widget *widget,
                                                   struct picoui_point point);
struct picoui_point picoui_widget_get_relative_pos(const struct picoui_widget *widget,
                                                   struct picoui_point point);
struct picoui_rect picoui_rect_align(struct picoui_rect parent,
                                     struct picoui_rect child,
                                     enum picoui_align x_align,
                                     enum picoui_align y_align);
struct picoui_rect picoui_rect_center(struct picoui_rect parent,
                                      struct picoui_rect child);
int picoui_vertical_grid_align_offset(struct picoui_rect widget,
                                      int current_offset,
                                      int item_count,
                                      int item_height,
                                      int space);
int picoui_focus_reset(struct picoui_app *app);
int picoui_focus_navigate(struct picoui_app *app, enum picoui_native_nav_dir dir);

#endif
