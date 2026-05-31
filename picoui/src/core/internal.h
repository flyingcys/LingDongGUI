#ifndef PICOUI_INTERNAL_H
#define PICOUI_INTERNAL_H

#include "backend.h"
#include "picoui/combo_box.h"
#include "picoui/calendar.h"
#include "picoui/keyboard.h"
#include "picoui/line_edit.h"
#include "picoui/message_box.h"
#include "picoui/graph.h"
#include "picoui/scroll_selecter.h"
#include "picoui/table.h"
#include "picoui/theme.h"

#define PICOUI_LAYOUT_MAX_TRACKS 16
#define PICOUI_LIST_MAX_ITEMS 16
#define PICOUI_GRAPH_MAX_SERIES 8
#define PICOUI_GRAPH_MAX_POINTS 32

typedef struct arm_2d_tile_t arm_2d_tile_t;

struct picoui_font;
struct picoui_message_box;

typedef void (*picoui_message_box_callback_t)(struct picoui_message_box *box, void *user_data);

enum picoui_focus_event {
    PICOUI_FOCUS_EVENT_NONE = 0,
    PICOUI_FOCUS_EVENT_ENTER,
    PICOUI_FOCUS_EVENT_LEAVE,
};

enum picoui_edit_result {
    PICOUI_EDIT_RESULT_NONE = 0,
    PICOUI_EDIT_RESULT_COMMIT,
    PICOUI_EDIT_RESULT_CANCEL,
};

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
    int has_focus;
    int focus_enter_count;
    int focus_leave_count;
    int focus_change_count;
    enum picoui_focus_event last_focus_event;
    enum picoui_edit_result pending_edit_result;
    enum picoui_edit_result last_edit_result;
};

struct picoui_app {
    void *backend_app;
    struct picoui_theme *theme;
    struct picoui_window *root_window;
    struct picoui_widget *focus_owner;
    struct picoui_widget *editing_owner;
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

struct picoui_progress_bar {
    struct picoui_widget widget;
    const char *id;
    int percent;
    int horizontal;
};

struct picoui_qrcode {
    struct picoui_widget widget;
    const char *id;
    const char *text;
};

struct picoui_progress_wheel {
    struct picoui_widget widget;
    const char *id;
    int percent;
};

struct picoui_date_time {
    struct picoui_widget widget;
    const char *id;
    const char *format;
    int year;
    int month;
    int day;
    int hour;
    int minute;
    int second;
};

struct picoui_calendar {
    struct picoui_widget widget;
    const char *id;
    const char *header_format;
    int year;
    int month;
    int day;
    int show_header;
    unsigned char grid_values[42];
    unsigned char grid_flags[42];
};

struct picoui_clock {
    struct picoui_widget widget;
    const char *id;
    int step_second;
};

struct picoui_list_item {
    const char *id;
    const char *text;
};

struct picoui_list {
    struct picoui_widget widget;
    const char *id;
    struct picoui_list_item items[PICOUI_LIST_MAX_ITEMS];
    const char *backend_item_ids[PICOUI_LIST_MAX_ITEMS];
    const unsigned char *backend_item_texts[PICOUI_LIST_MAX_ITEMS];
    int item_count;
    int selected_index;
    void (*cb)(struct picoui_list *list, int index, void *user_data);
    void *user_data;
};

struct picoui_message_box {
    struct picoui_widget widget;
    const char *id;
    const char *title;
    const char *message;
    const char *confirm_text;
    picoui_message_box_callback_t on_confirm;
    void *on_confirm_user_data;
};

struct picoui_text {
    struct picoui_widget widget;
    const char *id;
};

struct picoui_line_edit {
    struct picoui_widget widget;
    const char *id;
    enum picoui_line_edit_type type;
    unsigned int keyboard_binding;
    int editing;
    picoui_line_edit_finished_cb on_edit_finished;
    void *on_edit_finished_user_data;
};

struct picoui_keyboard {
    struct picoui_widget widget;
    const char *id;
};

struct picoui_combo_box {
    struct picoui_widget widget;
    const char *id;
    struct picoui_list_item items[PICOUI_LIST_MAX_ITEMS];
    const char *backend_item_ids[PICOUI_LIST_MAX_ITEMS];
    const unsigned char *backend_item_texts[PICOUI_LIST_MAX_ITEMS];
    int item_count;
    int selected_index;
    void (*cb)(struct picoui_combo_box *combo_box, int index, void *user_data);
    void *user_data;
};

struct picoui_scroll_selecter {
    struct picoui_widget widget;
    const char *id;
    struct picoui_list_item items[PICOUI_LIST_MAX_ITEMS];
    const char *backend_item_ids[PICOUI_LIST_MAX_ITEMS];
    const unsigned char *backend_item_texts[PICOUI_LIST_MAX_ITEMS];
    int item_count;
    int selected_index;
    int edit_mode;
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

struct picoui_graph {
    struct picoui_widget widget;
    const char *id;
    int series_max;
    int series_count;
    int series_point_counts[PICOUI_GRAPH_MAX_SERIES];
};

struct picoui_table {
    struct picoui_widget widget;
    const char *id;
    unsigned int keyboard_binding;
    int row_count;
    int column_count;
    int current_row;
    int current_column;
};

int picoui_widget_claim_focus(struct picoui_widget *widget);
int picoui_widget_release_focus(struct picoui_widget *widget);
int picoui_widget_is_focus_owner(const struct picoui_widget *widget);
int picoui_widget_mark_edit_result(struct picoui_widget *widget, enum picoui_edit_result result);
int picoui_widget_claim_editing(struct picoui_widget *widget);
int picoui_widget_release_editing(struct picoui_widget *widget);
int picoui_widget_is_editing_owner(const struct picoui_widget *widget);

#endif
