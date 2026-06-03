/*
 * Copyright (c) 2023-2026 flyingcys (flyingcys@gmail.com). All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef PICOUI_INTERNAL_H
#define PICOUI_INTERNAL_H

#include "backend.h"
#include "picoui/combo_box.h"
#include "picoui/canvas.h"
#include "picoui/calendar.h"
#include "picoui/keyboard.h"
#include "picoui/native.h"
#include "picoui/line_edit.h"
#include "picoui/message_box.h"
#include "picoui/graph.h"
#include "picoui/icon_slider.h"
#include "picoui/scroll_selecter.h"
#include "picoui/radial_menu.h"
#include "picoui/table.h"
#include "picoui/theme.h"

#define PICOUI_LAYOUT_MAX_TRACKS 16
#define PICOUI_LIST_MAX_ITEMS 16
#define PICOUI_GRAPH_MAX_SERIES 8
#define PICOUI_GRAPH_MAX_POINTS 32
#define PICOUI_CANVAS_MAX_COMMANDS 64

typedef struct arm_2d_tile_t arm_2d_tile_t;

struct picoui_font;
struct picoui_message_box;
struct kbBtnInfo_t;

typedef void (*picoui_message_box_callback_t)(struct picoui_message_box *box, void *user_data);

/**
 * @brief Native platform: align to ld grid
 *
 * @param[in] align align
 * @return 0 on success, -1 on failure
 */

int picoui_native_align_to_ld_grid(enum picoui_native_align align);

/**
 * @brief Native platform: nav dir to ld
 *
 * @param[in] dir dir
 * @return 0 on success, -1 on failure
 */

int picoui_native_nav_dir_to_ld(enum picoui_native_nav_dir dir);

/**
 * @brief Native platform: signal to ld
 *
 * @param[in] signal signal
 * @return 0 on success, -1 on failure
 */

int picoui_native_signal_to_ld(enum picoui_native_signal signal);

/**
 * @brief Native platform: readback policy to backend
 *
 * @param[in] policy policy
 * @return 0 on success, -1 on failure
 */

int picoui_native_readback_policy_to_backend(enum picoui_native_readback_policy policy);

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
    int opacity;
    const struct picoui_font *font;
    int visible;
    int enabled;
    int selectable;
    int selected;
    int corner;
    int flex_grow;
    int flex_new_track;
    int flex_min_width;
    int flex_min_height;
    int flex_max_width;
    int flex_max_height;
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
    int background_offset_x;
    int background_offset_y;
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

struct picoui_background {
    struct picoui_window window;
};

enum picoui_canvas_command_kind {
    PICOUI_CANVAS_COMMAND_FILL_RECT = 0,
    PICOUI_CANVAS_COMMAND_DRAW_LINE,
    PICOUI_CANVAS_COMMAND_DRAW_IMAGE,
    PICOUI_CANVAS_COMMAND_DRAW_IMAGE_SCALED,
    PICOUI_CANVAS_COMMAND_DRAW_TEXT,
};

struct picoui_canvas_command {
    enum picoui_canvas_command_kind kind;
    int x;
    int y;
    int width;
    int height;
    int x1;
    int y1;
    int line_size;
    unsigned int rgb0;
    unsigned int rgb1;
    int opacity0;
    int opacity1;
    float scale;
    enum picoui_align align;
    const char *text;
    struct picoui_image_source *source;
};

struct picoui_canvas {
    struct picoui_widget widget;
    const char *id;
    struct picoui_canvas_command commands[PICOUI_CANVAS_MAX_COMMANDS];
    int command_count;
};

struct picoui_keyboard_layout_entry {
    char *text;
    unsigned int key_code;
    unsigned int press_color;
    unsigned int release_color;
    int x;
    int y;
    int width;
    int height;
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

struct picoui_keyboard {
    struct picoui_widget widget;
    const char *id;
    const struct picoui_keyboard_button *buttons;
    struct picoui_keyboard_layout_entry *layout_entries;
    void *native_layout;
    int layout_count;
    picoui_keyboard_event_cb event_cb;
    void *event_user_data;
    picoui_keyboard_draw_cb draw_cb;
    void *draw_user_data;
    int draw_invocation_count;
    unsigned int last_draw_key_code;
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
    int inverted;
    unsigned int bg_color;
    unsigned int fg_color;
    unsigned int frame_color;
    int frame_color_size;
    struct picoui_image_source *bg_source;
    struct picoui_image_source *fg_source;
    struct picoui_image_source *frame_source;
};

struct picoui_animation {
    struct picoui_widget widget;
    const char *id;
    int width;
    int height;
    int period_ms;
    struct picoui_image_source *source;
};

struct picoui_arc {
    struct picoui_widget widget;
    const char *id;
    float bg_start_angle;
    float bg_end_angle;
    float fg_end_angle;
    float rotation_angle;
    struct picoui_image_source *quarter_source;
    unsigned int parent_color;
    unsigned int bg_color;
    unsigned int fg_color;
};

struct picoui_gauge {
    struct picoui_widget widget;
    const char *id;
    float angle;
    struct picoui_image_source *bg_source;
    struct picoui_image_source *pointer_source;
    int centre_offset_x;
    int centre_offset_y;
    unsigned int pointer_color;
    int auto_move;
};

struct picoui_list_item {
    const char *id;
    const char *text;
};

struct picoui_icon_slider {
    struct picoui_widget widget;
    const char *id;
    struct picoui_list_item items[PICOUI_LIST_MAX_ITEMS];
    struct picoui_image_source *item_sources[PICOUI_LIST_MAX_ITEMS];
    int item_count;
    int selected_index;
    int horizontal;
    int speed;
    int icon_width;
    int icon_space;
    int columns;
    int rows;
    int pages;
    void (*cb)(struct picoui_icon_slider *icon_slider, int index, void *user_data);
    void *user_data;
};

struct picoui_radial_menu {
    struct picoui_widget widget;
    const char *id;
    struct picoui_list_item items[PICOUI_LIST_MAX_ITEMS];
    struct picoui_image_source *item_sources[PICOUI_LIST_MAX_ITEMS];
    int item_count;
    int selected_index;
    int x_axis;
    int y_axis;
    int item_max;
    void (*cb)(struct picoui_radial_menu *radial_menu, int index, void *user_data);
    void *user_data;
};

struct picoui_qrcode {
    struct picoui_widget widget;
    const char *id;
    const char *text;
    unsigned int qr_color;
    unsigned int bg_color;
    int ecc;
    int max_version;
    int zoom;
};

struct picoui_progress_wheel {
    struct picoui_widget widget;
    const char *id;
    int percent;
    unsigned int wheel_color;
    unsigned int dot_color;
    int dot_enabled;
};

struct picoui_date_time {
    struct picoui_widget widget;
    const char *id;
    const char *format;
    unsigned int text_color;
    unsigned int bg_color;
    enum picoui_align align;
    int transparent;
    int year;
    int month;
    int day;
    int hour;
    int minute;
    int second;
    int use_system_time;
};

struct picoui_calendar {
    struct picoui_widget widget;
    const char *id;
    const char *header_format;
    int year;
    int month;
    int day;
    int show_header;
    int use_system_date;
    unsigned char grid_values[42];
    unsigned char grid_flags[42];
};

struct picoui_clock {
    struct picoui_widget widget;
    const char *id;
    struct picoui_image_source *background_source;
    struct picoui_image_source *hour_pointer_source;
    struct picoui_image_source *minute_pointer_source;
    struct picoui_image_source *second_pointer_source;
    unsigned int mask_color;
    float hour_anchor_x;
    float hour_anchor_y;
    float minute_anchor_x;
    float minute_anchor_y;
    float second_anchor_x;
    float second_anchor_y;
    int use_system_time;
    int step_second;
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
    const char *buttons[PICOUI_LIST_MAX_ITEMS];
    int button_count;
    unsigned int title_color;
    unsigned int message_color;
    unsigned int button_color;
    unsigned int release_color;
    unsigned int press_color;
    unsigned int bg_color;
    picoui_message_box_callback_t on_confirm;
    void *on_confirm_user_data;
    picoui_message_box_indexed_callback_t on_confirm_indexed;
    void *on_confirm_indexed_user_data;
};

struct picoui_text {
    struct picoui_widget widget;
    const char *id;
};

struct picoui_line_edit {
    struct picoui_widget widget;
    const char *id;
    enum picoui_align align;
    enum picoui_line_edit_type type;
    unsigned int keyboard_binding;
    int editing;
    picoui_line_edit_finished_cb on_edit_finished;
    void *on_edit_finished_user_data;
};

struct picoui_combo_box {
    struct picoui_widget widget;
    const char *id;
    struct picoui_list_item items[PICOUI_LIST_MAX_ITEMS];
    const char *backend_item_ids[PICOUI_LIST_MAX_ITEMS];
    const unsigned char *backend_item_texts[PICOUI_LIST_MAX_ITEMS];
    int item_count;
    int item_max;
    int selected_index;
    struct picoui_image_source *dropdown_source;
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
    int transparent;
    int speed;
    struct picoui_image_source *bg_source;
    struct picoui_image_source *indicator_source;
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
    int x_axis;
    int y_axis;
    int axis_offset;
    int frame_space;
    int grid_offset;
    struct picoui_image_source *point_mask_source;
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

/**
 * @brief Claim input focus
 *
 * @param[in] widget Widget instance
 * @return 0 on success, -1 on failure
 */

int picoui_widget_claim_focus(struct picoui_widget *widget);

/**
 * @brief Widget: release focus
 *
 * @param[in] widget Widget instance
 * @return 0 on success, -1 on failure
 */

int picoui_widget_release_focus(struct picoui_widget *widget);

/**
 * @brief Widget: is focus owner
 *
 * @param[in] widget Widget instance
 * @return 0 on success, -1 on failure
 */

int picoui_widget_is_focus_owner(const struct picoui_widget *widget);

/**
 * @brief Widget: mark edit result
 *
 * @param[in] widget Widget instance
 * @param[in] result result
 * @return 0 on success, -1 on failure
 */

int picoui_widget_mark_edit_result(struct picoui_widget *widget, enum picoui_edit_result result);

/**
 * @brief Widget: claim editing
 *
 * @param[in] widget Widget instance
 * @return 0 on success, -1 on failure
 */

int picoui_widget_claim_editing(struct picoui_widget *widget);

/**
 * @brief Widget: release editing
 *
 * @param[in] widget Widget instance
 * @return 0 on success, -1 on failure
 */

int picoui_widget_release_editing(struct picoui_widget *widget);

/**
 * @brief Widget: is editing owner
 *
 * @param[in] widget Widget instance
 * @return 0 on success, -1 on failure
 */

int picoui_widget_is_editing_owner(const struct picoui_widget *widget);

#endif
