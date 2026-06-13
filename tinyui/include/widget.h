#ifndef TINYUI_WIDGET_H
#define TINYUI_WIDGET_H

#include "layout.h"
#include "native.h"
#include "theme.h"

struct tinyui_widget;
struct tinyui_app;
struct tinyui_font;
typedef struct tinyui_widget tinyui_obj_t;

enum tinyui_font_kind {
    TINYUI_FONT_KIND_FAMILY = 0,
    TINYUI_FONT_KIND_VRES = 1,
};

typedef void (*tinyui_value_changed_cb)(struct tinyui_widget *widget,
                                        int value,
                                        void *user_data);
typedef void (*tinyui_event_cb)(struct tinyui_widget *widget, void *user_data);

struct tinyui_font {
    const char *family;
    int size;
    enum tinyui_font_kind kind;
    unsigned int vres_addr;
};

struct tinyui_point {
    int x;
    int y;
};

struct tinyui_size {
    int width;
    int height;
};

struct tinyui_rect {
    int x;
    int y;
    int width;
    int height;
};

enum tinyui_widget_type {
    TINYUI_WIDGET_TYPE_UNKNOWN = 0,
    TINYUI_WIDGET_TYPE_BACKGROUND,
    TINYUI_WIDGET_TYPE_WINDOW,
    TINYUI_WIDGET_TYPE_BUTTON,
    TINYUI_WIDGET_TYPE_IMAGE,
    TINYUI_WIDGET_TYPE_TEXT,
    TINYUI_WIDGET_TYPE_LINE_EDIT,
    TINYUI_WIDGET_TYPE_GRAPH,
    TINYUI_WIDGET_TYPE_CHECKBOX,
    TINYUI_WIDGET_TYPE_SLIDER,
    TINYUI_WIDGET_TYPE_SWITCH,
    TINYUI_WIDGET_TYPE_PROGRESS_BAR,
    TINYUI_WIDGET_TYPE_GAUGE,
    TINYUI_WIDGET_TYPE_QRCODE,
    TINYUI_WIDGET_TYPE_DATE_TIME,
    TINYUI_WIDGET_TYPE_ICON_SLIDER,
    TINYUI_WIDGET_TYPE_COMBO_BOX,
    TINYUI_WIDGET_TYPE_ARC,
    TINYUI_WIDGET_TYPE_RADIAL_MENU,
    TINYUI_WIDGET_TYPE_SCROLL_SELECTER,
    TINYUI_WIDGET_TYPE_LABEL,
    TINYUI_WIDGET_TYPE_TABLE,
    TINYUI_WIDGET_TYPE_KEYBOARD,
    TINYUI_WIDGET_TYPE_ANIMATION,
    TINYUI_WIDGET_TYPE_LIST,
    TINYUI_WIDGET_TYPE_MESSAGE_BOX,
    TINYUI_WIDGET_TYPE_CALENDAR,
    TINYUI_WIDGET_TYPE_PROGRESS_WHEEL,
    TINYUI_WIDGET_TYPE_CLOCK,
    TINYUI_WIDGET_TYPE_CANVAS,
};

int tinyui_widget_set_pos(struct tinyui_widget *widget, int x, int y);
int tinyui_widget_set_size(struct tinyui_widget *widget, int width, int height);
int tinyui_widget_set_text(struct tinyui_widget *widget, const char *text);
int tinyui_widget_set_style_class(struct tinyui_widget *widget, const char *style_class);
int tinyui_widget_set_user_data(struct tinyui_widget *widget, void *user_data);
int tinyui_widget_set_bg_color(struct tinyui_widget *widget, unsigned int rgb);
int tinyui_widget_set_text_color(struct tinyui_widget *widget, unsigned int rgb);
int tinyui_widget_set_border_color(struct tinyui_widget *widget, unsigned int rgb);
int tinyui_widget_set_radius(struct tinyui_widget *widget, int radius);
int tinyui_widget_set_padding(struct tinyui_widget *widget, int padding);
int tinyui_widget_set_center(struct tinyui_widget *widget);
int tinyui_widget_set_visible(struct tinyui_widget *widget, int visible);
int tinyui_widget_is_hidden(struct tinyui_widget *widget);
int tinyui_widget_set_opacity(struct tinyui_widget *widget, int opacity);
int tinyui_widget_set_selectable(struct tinyui_widget *widget, int selectable);
int tinyui_widget_set_selected(struct tinyui_widget *widget, int selected);
int tinyui_widget_set_corner(struct tinyui_widget *widget, int corner);
int tinyui_widget_set_enabled(struct tinyui_widget *widget, int enabled);
int tinyui_widget_set_flex_grow(struct tinyui_widget *widget, int grow);
int tinyui_widget_set_flex_new_track(struct tinyui_widget *widget, int new_track);
int tinyui_widget_set_flex_min_width(struct tinyui_widget *widget, int min_width);
int tinyui_widget_set_flex_min_height(struct tinyui_widget *widget, int min_height);
int tinyui_widget_set_flex_max_width(struct tinyui_widget *widget, int max_width);
int tinyui_widget_set_flex_max_height(struct tinyui_widget *widget, int max_height);
int tinyui_widget_set_ignore_layout(struct tinyui_widget *widget, int ignore_layout);
int tinyui_widget_set_grid_cell(struct tinyui_widget *widget,
                                int col,
                                int row,
                                int col_span,
                                int row_span,
                                enum tinyui_align x_align,
                                enum tinyui_align y_align);
int tinyui_widget_remove_from_parent(struct tinyui_widget *widget);
int tinyui_widget_destroy(struct tinyui_widget *widget);
int tinyui_widget_get_x(const struct tinyui_widget *widget);
int tinyui_widget_get_y(const struct tinyui_widget *widget);
int tinyui_widget_get_width(const struct tinyui_widget *widget);
int tinyui_widget_get_height(const struct tinyui_widget *widget);
int tinyui_widget_get_visible(const struct tinyui_widget *widget);
int tinyui_widget_get_opacity(const struct tinyui_widget *widget);
int tinyui_widget_get_selectable(const struct tinyui_widget *widget);
int tinyui_widget_get_selected(const struct tinyui_widget *widget);
int tinyui_widget_get_corner(const struct tinyui_widget *widget);
struct tinyui_widget *tinyui_widget_get_parent(const struct tinyui_widget *widget);
struct tinyui_widget *tinyui_widget_get_first_child(const struct tinyui_widget *widget);
struct tinyui_widget *tinyui_widget_get_next_sibling(const struct tinyui_widget *widget);
struct tinyui_widget *tinyui_widget_get_root(const struct tinyui_widget *widget);
int tinyui_widget_get_child_count(const struct tinyui_widget *widget);
int tinyui_widget_get_name_id(const struct tinyui_widget *widget);
struct tinyui_widget *tinyui_widget_find_by_name_id(const struct tinyui_widget *root, int name_id);
enum tinyui_widget_type tinyui_widget_get_type(const struct tinyui_widget *widget);
struct tinyui_point tinyui_widget_get_absolute_pos(const struct tinyui_widget *widget,
                                                   struct tinyui_point point);
struct tinyui_point tinyui_widget_get_relative_pos(const struct tinyui_widget *widget,
                                                   struct tinyui_point point);
struct tinyui_rect tinyui_rect_align(struct tinyui_rect parent,
                                     struct tinyui_rect child,
                                     enum tinyui_align x_align,
                                     enum tinyui_align y_align);
struct tinyui_rect tinyui_rect_center(struct tinyui_rect parent,
                                      struct tinyui_rect child);
int tinyui_vertical_grid_align_offset(struct tinyui_rect widget,
                                      int current_offset,
                                      int item_count,
                                      int item_height,
                                      int space);
int tinyui_focus_reset(struct tinyui_app *app);
int tinyui_focus_navigate(struct tinyui_app *app, enum tinyui_native_nav_dir dir);

#endif
