#ifndef PICOUI_BACKEND_LDGUI_H
#define PICOUI_BACKEND_LDGUI_H

#include <stdint.h>

#include "picoui/widget.h"

struct picoui_widget;
struct picoui_message_box;
struct picoui_app;
struct picoui_theme;
struct picoui_window;
struct picoui_image_source;
struct picoui_list;
struct picoui_combo_box;
struct picoui_icon_slider;
struct picoui_radial_menu;
struct picoui_scroll_selecter;
struct picoui_table;
struct picoui_calendar;
struct ld_scene_t;
enum picoui_line_edit_type;

#define PICOUI_BACKEND_LAYOUT_MAX_TRACKS 16
#define PICOUI_BACKEND_LIST_MAX_ITEMS 16

enum picoui_backend_widget_kind {
    PICOUI_BACKEND_WIDGET_WINDOW = 0,
    PICOUI_BACKEND_WIDGET_LABEL,
    PICOUI_BACKEND_WIDGET_BUTTON,
    PICOUI_BACKEND_WIDGET_CHECKBOX,
    PICOUI_BACKEND_WIDGET_SWITCH,
    PICOUI_BACKEND_WIDGET_SLIDER,
    PICOUI_BACKEND_WIDGET_ARC,
    PICOUI_BACKEND_WIDGET_GAUGE,
    PICOUI_BACKEND_WIDGET_ICON_SLIDER,
    PICOUI_BACKEND_WIDGET_RADIAL_MENU,
    PICOUI_BACKEND_WIDGET_PROGRESS_BAR,
    PICOUI_BACKEND_WIDGET_QRCODE,
    PICOUI_BACKEND_WIDGET_PROGRESS_WHEEL,
    PICOUI_BACKEND_WIDGET_LIST,
    PICOUI_BACKEND_WIDGET_MESSAGE_BOX,
    PICOUI_BACKEND_WIDGET_DATE_TIME,
    PICOUI_BACKEND_WIDGET_CLOCK,
    PICOUI_BACKEND_WIDGET_TEXT,
    PICOUI_BACKEND_WIDGET_KEYBOARD,
    PICOUI_BACKEND_WIDGET_COMBO_BOX,
    PICOUI_BACKEND_WIDGET_SCROLL_SELECTER,
    PICOUI_BACKEND_WIDGET_TABLE,
    PICOUI_BACKEND_WIDGET_GRAPH,
    PICOUI_BACKEND_WIDGET_IMAGE,
    PICOUI_BACKEND_WIDGET_CALENDAR,
};

enum picoui_backend_signal {
    PICOUI_BACKEND_SIGNAL_NONE = 0,
    PICOUI_BACKEND_SIGNAL_VALUE_CHANGED,
    PICOUI_BACKEND_SIGNAL_PRESSED,
    PICOUI_BACKEND_SIGNAL_RELEASED,
};

enum picoui_backend_data_truth_policy {
    PICOUI_BACKEND_DATA_TRUTH_NOT_APPLICABLE = 0,
    PICOUI_BACKEND_DATA_TRUTH_BACKEND_VALUE,
};

enum picoui_backend_data_value_source {
    PICOUI_BACKEND_DATA_SOURCE_NONE = 0,
    PICOUI_BACKEND_DATA_SOURCE_SETTER,
    PICOUI_BACKEND_DATA_SOURCE_NATIVE_EVENT,
};

struct picoui_backend_layout_window_state {
    enum picoui_flex_flow flex_flow;
    enum picoui_align flex_main_align;
    enum picoui_align flex_cross_align;
    enum picoui_align flex_track_align;
    int padding;
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

enum picoui_backend_runtime_evidence_flags {
    PICOUI_BACKEND_EVIDENCE_EXCLUDE_FORMAL_MAPPING = 1 << 0,
    PICOUI_BACKEND_EVIDENCE_ALLOW_SMOKE_LAYOUT = 1 << 1,
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
    const char *list_item_ids[PICOUI_BACKEND_LIST_MAX_ITEMS];
    int list_item_count;
    unsigned int data_model_identity;
    unsigned int data_model_epoch;
    enum picoui_backend_data_truth_policy data_truth_policy;
    enum picoui_backend_data_value_source last_data_source;
    int edit_result_on_finish;
    struct picoui_backend_layout_window_state window_layout;
    struct picoui_backend_layout_child_state child_layout;
    unsigned int runtime_evidence_flags;
    int open;
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
void *picoui_backend_create_arc(void *parent, const char *id);
void *picoui_backend_create_gauge(void *parent, const char *id);
void *picoui_backend_create_icon_slider(void *parent,
                                        const char *id,
                                        int width,
                                        int height,
                                        int icon_width,
                                        int icon_space,
                                        int columns,
                                        int rows,
                                        int pages);
void *picoui_backend_create_radial_menu(void *parent,
                                        const char *id,
                                        int width,
                                        int height,
                                        int x_axis,
                                        int y_axis,
                                        int item_max);
void *picoui_backend_create_progress_bar(void *parent, const char *id);
void *picoui_backend_create_qrcode(void *parent, const char *id);
void *picoui_backend_create_progress_wheel(void *parent, const char *id);
void *picoui_backend_create_list(void *parent, const char *id);
void *picoui_backend_create_message_box(void *parent, const char *id);
void *picoui_backend_create_date_time(void *parent, const char *id);
void *picoui_backend_create_clock(void *parent, const char *id);
void *picoui_backend_create_text(void *parent, const char *id);
void *picoui_backend_create_keyboard(void *parent, const char *id);
void *picoui_backend_create_line_edit(void *parent, const char *id);
void *picoui_backend_create_combo_box(void *parent, const char *id);
void *picoui_backend_create_scroll_selecter(void *parent, const char *id);
void *picoui_backend_create_table(void *parent, const char *id, int rows, int columns);
void *picoui_backend_create_graph(void *parent, const char *id, int series_max);
void *picoui_backend_create_image(void *parent, const char *id);
void *picoui_backend_create_calendar(void *parent, const char *id);
int picoui_backend_calendar_set_date(void *backend_widget, int year, int month, int day);
int picoui_backend_calendar_get_date(void *backend_widget, int *year, int *month, int *day);
int picoui_backend_calendar_set_header_visible(void *backend_widget, int visible);
int picoui_backend_calendar_get_header_visible(void *backend_widget);
int picoui_backend_calendar_set_header_format(void *backend_widget, const char *format);
const char *picoui_backend_calendar_get_header_format(void *backend_widget);
int picoui_backend_calendar_get_grid_value(void *backend_widget, int week, int weekday);
int picoui_backend_calendar_is_current_month_cell(void *backend_widget, int week, int weekday);
int picoui_backend_message_box_set_on_confirm(struct picoui_message_box *box);
int picoui_backend_set_text(void *backend_widget, const char *text);
int picoui_backend_line_edit_set_text(void *backend_widget, const char *text);
const char *picoui_backend_line_edit_get_text(void *backend_widget);
int picoui_backend_line_edit_set_type(void *backend_widget, enum picoui_line_edit_type type);
int picoui_backend_line_edit_get_type(void *backend_widget, enum picoui_line_edit_type *type);
int picoui_backend_line_edit_set_keyboard_binding(void *backend_widget,
                                                  unsigned int keyboard_binding);
int picoui_backend_line_edit_get_keyboard_binding(void *backend_widget,
                                                  unsigned int *keyboard_binding);
int picoui_backend_line_edit_bind_host(void *backend_widget);
int picoui_backend_line_edit_get_editing(void *backend_widget, int *editing);
int picoui_backend_keyboard_input_ascii(void *backend_widget, unsigned int ascii);
int picoui_backend_keyboard_navigate(void *backend_widget, int direction);
int picoui_backend_keyboard_click(void *backend_widget);
int picoui_backend_keyboard_exit(void *backend_widget);
int picoui_backend_combo_box_set_items(void *backend_widget,
                                       const char *const *item_ids,
                                       const unsigned char *const *items,
                                       int item_count);
int picoui_backend_combo_box_set_selected_index(void *backend_widget, int index);
int picoui_backend_combo_box_get_selected_index(void *backend_widget);
int picoui_backend_combo_box_sync_selected_index(struct picoui_combo_box *combo_box,
                                                 int *selected_index_out);
int picoui_backend_combo_box_bind_host(void *backend_widget);
int picoui_backend_combo_box_get_open(void *backend_widget, int *is_open);
int picoui_backend_icon_slider_add_item(void *backend_widget, const char *id, const char *text);
int picoui_backend_icon_slider_set_selected_index(void *backend_widget, int index);
int picoui_backend_icon_slider_get_selected_index(void *backend_widget);
int picoui_backend_icon_slider_set_horizontal(void *backend_widget, int horizontal);
int picoui_backend_icon_slider_get_horizontal(void *backend_widget, int *horizontal);
int picoui_backend_icon_slider_bind_host(void *backend_widget);
int picoui_backend_radial_menu_add_item(void *backend_widget, const char *id);
int picoui_backend_radial_menu_set_selected_index(void *backend_widget, int index);
int picoui_backend_radial_menu_get_selected_index(void *backend_widget);
int picoui_backend_radial_menu_offset_selection(void *backend_widget, int offset);
int picoui_backend_radial_menu_bind_host(void *backend_widget);
int picoui_backend_scroll_selecter_set_items(void *backend_widget,
                                             const char *const *item_ids,
                                             const unsigned char *const *items,
                                             int item_count);
int picoui_backend_scroll_selecter_set_selected_index(void *backend_widget, int index);
int picoui_backend_scroll_selecter_get_selected_index(void *backend_widget);
int picoui_backend_scroll_selecter_sync_selected_index(struct picoui_scroll_selecter *scroll_selecter,
                                                       int *selected_index_out);
int picoui_backend_scroll_selecter_set_edit_mode(void *backend_widget, int is_edit);
int picoui_backend_scroll_selecter_get_edit_mode(void *backend_widget, int *is_edit);
int picoui_backend_table_set_keyboard_binding(void *backend_widget, unsigned int keyboard_binding);
int picoui_backend_table_get_keyboard_binding(void *backend_widget, unsigned int *keyboard_binding);
int picoui_backend_table_set_cell_text(void *backend_widget, int row, int column, const char *text);
const char *picoui_backend_table_get_cell_text(void *backend_widget, int row, int column);
int picoui_backend_table_set_cell_editable(void *backend_widget,
                                           int row,
                                           int column,
                                           int editable,
                                           unsigned int text_max);
int picoui_backend_table_set_current_cell(void *backend_widget, int row, int column);
int picoui_backend_table_sync_current_cell(struct picoui_table *table, int *row_out, int *column_out);
int picoui_backend_table_bind_host(void *backend_widget);
int picoui_backend_graph_add_series(void *backend_widget,
                                    unsigned int series_color,
                                    int line_size,
                                    int point_max);
int picoui_backend_graph_set_value(void *backend_widget, int series_index, int value_index, int value);
int picoui_backend_graph_move_add(void *backend_widget, int series_index, int value);
int picoui_backend_graph_get_series_count(void *backend_widget);
int picoui_backend_graph_get_value(void *backend_widget, int series_index, int value_index);
int picoui_backend_list_set_items(void *backend_widget,
                                  const char *const *item_ids,
                                  const unsigned char *const *items,
                                  int item_count);
int picoui_backend_list_set_selected_index(void *backend_widget, int index);
int picoui_backend_list_get_selected_index(void *backend_widget);
int picoui_backend_list_sync_selected_index(struct picoui_list *list, int *selected_index_out);
int picoui_backend_widget_set_style_class(void *backend_widget, const char *style_class);
int picoui_backend_widget_set_font(void *backend_widget, const void *font);
int picoui_backend_widget_set_user_data(void *backend_widget, void *user_data);
int picoui_backend_widget_set_padding(void *backend_widget, int padding);
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
int picoui_backend_widget_claim_focus(void *backend_widget);
int picoui_backend_widget_release_focus(void *backend_widget);
void picoui_backend_widget_init_data_model(struct picoui_backend_widget *backend);

#endif
