#ifndef PICOUI_BACKEND_LDGUI_H
#define PICOUI_BACKEND_LDGUI_H

#include <stdint.h>

#include "picoui/widget.h"

struct picoui_widget;
struct picoui_app;
struct picoui_theme;
struct picoui_window;
struct picoui_image_source;
struct ld_scene_t;

#define PICOUI_BACKEND_LAYOUT_MAX_TRACKS 16

enum picoui_backend_widget_kind {
    PICOUI_BACKEND_WIDGET_WINDOW = 0,
    PICOUI_BACKEND_WIDGET_LABEL,
    PICOUI_BACKEND_WIDGET_BUTTON,
    PICOUI_BACKEND_WIDGET_CHECKBOX,
    PICOUI_BACKEND_WIDGET_SWITCH,
    PICOUI_BACKEND_WIDGET_SLIDER,
    PICOUI_BACKEND_WIDGET_LIST,
    PICOUI_BACKEND_WIDGET_TEXT,
    PICOUI_BACKEND_WIDGET_IMAGE,
};

enum picoui_backend_signal {
    PICOUI_BACKEND_SIGNAL_NONE = 0,
    PICOUI_BACKEND_SIGNAL_VALUE_CHANGED,
    PICOUI_BACKEND_SIGNAL_PRESSED,
    PICOUI_BACKEND_SIGNAL_RELEASED,
};

struct picoui_backend_layout_window_state {
    enum picoui_flex_flow flex_flow;
    enum picoui_align flex_main_align;
    enum picoui_align flex_cross_align;
    enum picoui_align flex_track_align;
    int flex_item_gap;
    int flex_track_gap;
    int16_t grid_cols[PICOUI_BACKEND_LAYOUT_MAX_TRACKS];
    int16_t grid_rows[PICOUI_BACKEND_LAYOUT_MAX_TRACKS];
    int grid_col_count;
    int grid_row_count;
    int grid_row_gap;
    int grid_col_gap;
    enum picoui_align grid_col_align;
    enum picoui_align grid_row_align;
};

struct picoui_backend_layout_child_state {
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

struct picoui_backend_widget {
    struct picoui_app *owner;
    struct picoui_widget *host_widget;
    struct picoui_backend_widget *root;
    struct picoui_backend_widget *parent;
    struct picoui_backend_widget *first_child;
    struct picoui_backend_widget *next_sibling;
    const char *id;
    enum picoui_backend_widget_kind kind;
    const char *text;
    const char *style_class;
    const void *font;
    void *user_data;
    struct ld_scene_t *ld_event_bridge_scene;
    void *ld_event_bridge_sender;
    struct picoui_backend_widget *ld_event_bridge_next;
    struct picoui_image_source *image_source;
    int value;
    enum picoui_backend_signal last_signal;
    int dispatch_count;
    struct picoui_theme *theme;
    void *ld_widget;
    uint16_t ld_name_id;
    struct picoui_backend_layout_window_state window_layout;
    struct picoui_backend_layout_child_state child_layout;
};

struct picoui_backend_app_state {
    struct picoui_theme *theme;
    struct ld_scene_t *ld_scene;
    uint16_t next_ld_name_id;
    void *runtime_state;
};

int picoui_backend_app_init(struct picoui_app *app);
int picoui_backend_app_run(struct picoui_app *app, struct picoui_window *window);
void picoui_backend_app_shutdown(struct picoui_app *app);
int picoui_backend_apply_theme(struct picoui_app *app, struct picoui_theme *theme);
int picoui_backend_widget_apply_style(void *backend_widget,
                                      enum picoui_part part,
                                      enum picoui_state state,
                                      unsigned int bg_color,
                                      unsigned int text_color,
                                      unsigned int border_color);
int picoui_backend_widget_is_kind(const void *backend_widget,
                                  enum picoui_backend_widget_kind kind);
struct picoui_app *picoui_backend_widget_get_owner(void *backend_widget);
struct picoui_backend_widget *picoui_backend_widget_get_root(void *backend_widget);
int picoui_backend_widget_attach_child(void *parent, void *child);
void *picoui_backend_create_window(struct picoui_app *app, const char *id);
void *picoui_backend_create_label(void *parent, const char *id);
void *picoui_backend_create_button(void *parent, const char *id);
void *picoui_backend_create_checkbox(void *parent, const char *id);
void *picoui_backend_create_switch(void *parent, const char *id);
void *picoui_backend_create_slider(void *parent, const char *id);
void *picoui_backend_create_list(void *parent, const char *id);
void *picoui_backend_create_text(void *parent, const char *id);
void *picoui_backend_create_image(void *parent, const char *id);
int picoui_backend_set_text(void *backend_widget, const char *text);
int picoui_backend_list_set_items(void *backend_widget,
                                  const unsigned char *const *items,
                                  int item_count);
int picoui_backend_list_set_selected_index(void *backend_widget, int index);
int picoui_backend_list_get_selected_index(void *backend_widget);
int picoui_backend_widget_set_style_class(void *backend_widget, const char *style_class);
int picoui_backend_widget_set_font(void *backend_widget, const void *font);
int picoui_backend_widget_set_user_data(void *backend_widget, void *user_data);
int picoui_backend_widget_bind_ld_event_bridge(void *backend_widget,
                                               struct ld_scene_t *scene,
                                               void *sender);
int picoui_backend_widget_bind_host(void *backend_widget,
                                    struct picoui_widget *widget);
int picoui_backend_set_image_source(void *backend_widget, struct picoui_image_source *source);
int picoui_backend_window_set_flex_flow(struct picoui_window *window, enum picoui_flex_flow flow);
int picoui_backend_window_set_flex_align(struct picoui_window *window,
                                         enum picoui_align main_align,
                                         enum picoui_align cross_align,
                                         enum picoui_align track_align);
int picoui_backend_window_set_flex_gap(struct picoui_window *window, int item_gap, int track_gap);
int picoui_backend_window_set_grid_columns(struct picoui_window *window, const int *tracks, int count);
int picoui_backend_window_set_grid_rows(struct picoui_window *window, const int *tracks, int count);
int picoui_backend_window_set_grid_gap(struct picoui_window *window, int row_gap, int col_gap);
int picoui_backend_window_set_grid_align(struct picoui_window *window,
                                         enum picoui_align col_align,
                                         enum picoui_align row_align);
int picoui_backend_widget_set_flex_grow(struct picoui_widget *widget, int grow);
int picoui_backend_widget_set_flex_new_track(struct picoui_widget *widget, int new_track);
int picoui_backend_widget_set_ignore_layout(struct picoui_widget *widget, int ignore_layout);
int picoui_backend_widget_set_grid_cell(struct picoui_widget *widget,
                                        int col,
                                        int row,
                                        int col_span,
                                        int row_span,
                                        enum picoui_align x_align,
                                        enum picoui_align y_align);
void picoui_backend_emit_value_changed(picoui_value_changed_cb cb,
                                       struct picoui_widget *widget,
                                       int value,
                                       void *user_data);
void picoui_backend_emit_event(picoui_event_cb cb,
                               struct picoui_widget *widget,
                               void *user_data);
int picoui_backend_widget_dispatch_signal(void *backend_widget,
                                          enum picoui_backend_signal signal,
                                          int value,
                                          picoui_value_changed_cb cb,
                                          struct picoui_widget *widget,
                                          void *user_data);
int picoui_backend_widget_dispatch_event(void *backend_widget,
                                         enum picoui_backend_signal signal,
                                         picoui_event_cb cb,
                                         struct picoui_widget *widget,
                                         void *user_data);
int picoui_backend_widget_dispatch_native_signal(void *backend_widget,
                                                 uint32_t native_signal,
                                                 uint64_t native_value);
int picoui_backend_widget_update_value(void *backend_widget,
                                       int value,
                                       picoui_value_changed_cb cb,
                                       struct picoui_widget *widget,
                                       void *user_data);
void picoui_backend_emit_clicked(picoui_event_cb cb,
                                 struct picoui_widget *widget,
                                 void *user_data);

#endif
