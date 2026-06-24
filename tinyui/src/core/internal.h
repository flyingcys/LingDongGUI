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

#ifndef TINYUI_INTERNAL_H
#define TINYUI_INTERNAL_H

#include <stddef.h> /* size_t */

/* ── enum tinyui_backend_widget_kind — canonical location ─────────────────
 * Declared here so that struct tinyui_widget (below) can embed it as a
 * field. (Phase C3-T4: runtime_internal.h was deleted; its surviving
 * shared enums/constants were folded into this header.)
 * ──────────────────────────────────────────────────────────────────────── */
#define TINYUI_BACKEND_WIDGET_KIND_DEFINED

enum tinyui_backend_widget_kind {
    TINYUI_BACKEND_WIDGET_WINDOW = 0,
    TINYUI_BACKEND_WIDGET_BACKGROUND,
    TINYUI_BACKEND_WIDGET_LABEL,
    TINYUI_BACKEND_WIDGET_BUTTON,
    TINYUI_BACKEND_WIDGET_CHECKBOX,
    TINYUI_BACKEND_WIDGET_SWITCH,
    TINYUI_BACKEND_WIDGET_SLIDER,
    TINYUI_BACKEND_WIDGET_ARC,
    TINYUI_BACKEND_WIDGET_GAUGE,
    TINYUI_BACKEND_WIDGET_ICON_SLIDER,
    TINYUI_BACKEND_WIDGET_RADIAL_MENU,
    TINYUI_BACKEND_WIDGET_PROGRESS_BAR,
    TINYUI_BACKEND_WIDGET_QRCODE,
    TINYUI_BACKEND_WIDGET_PROGRESS_WHEEL,
    TINYUI_BACKEND_WIDGET_ANIMATION,
    TINYUI_BACKEND_WIDGET_LIST,
    TINYUI_BACKEND_WIDGET_MESSAGE_BOX,
    TINYUI_BACKEND_WIDGET_DATE_TIME,
    TINYUI_BACKEND_WIDGET_CLOCK,
    TINYUI_BACKEND_WIDGET_TEXT,
    TINYUI_BACKEND_WIDGET_KEYBOARD,
    TINYUI_BACKEND_WIDGET_COMBO_BOX,
    TINYUI_BACKEND_WIDGET_SCROLL_SELECTER,
    TINYUI_BACKEND_WIDGET_TABLE,
    TINYUI_BACKEND_WIDGET_GRAPH,
    TINYUI_BACKEND_WIDGET_IMAGE,
    TINYUI_BACKEND_WIDGET_CALENDAR,
    TINYUI_BACKEND_WIDGET_CANVAS,
};

/* ── shared backend signals (folded from runtime_internal.h) ─────────────── */
enum tinyui_backend_signal {
    TINYUI_BACKEND_SIGNAL_NONE = 0,
    TINYUI_BACKEND_SIGNAL_VALUE_CHANGED,
    TINYUI_BACKEND_SIGNAL_PRESSED,
    TINYUI_BACKEND_SIGNAL_RELEASED,
};

/* ── shared constants (folded from runtime_internal.h) ───────────────────── */
#define TINYUI_BACKEND_LAYOUT_MAX_TRACKS 16
#define TINYUI_BACKEND_LIST_MAX_ITEMS 16

struct ld_scene_t;

#include "app.h"
#include "../../../src/misc/xBtnAction.h"

/* Forward declarations for widget types used in internal function declarations.
 * These must appear before any function declaration that uses them as parameter
 * types, otherwise C creates file-scope-local struct tags that conflict with
 * the real definitions later in this header. */
struct tinyui_animation;
struct tinyui_message_box;
struct tinyui_list;
struct tinyui_combo_box;
struct tinyui_scroll_selecter;
struct tinyui_table;
struct tinyui_icon_slider;
struct tinyui_radial_menu;
struct tinyui_checkbox;
struct tinyui_switch;
struct tinyui_slider;
struct tinyui_progress_wheel;
struct tinyui_date_time;
struct tinyui_clock;
struct tinyui_arc;
struct tinyui_gauge;
struct tinyui_progress_wheel_props;
struct tinyui_calendar;
struct tinyui_canvas;
#include "combo_box.h"
#include "canvas.h"
#include "calendar.h"
#include "keyboard.h"
#include "native.h"
#include "display.h"
#include "indev.h"
#include "osal.h"
#include "tick.h"
#include "line_edit.h"
#include "message_box.h"
#include "graph.h"
#include "icon_slider.h"
#include "scroll_selecter.h"
#include "radial_menu.h"
#include "table.h"
#include "theme.h"

#define TINYUI_LAYOUT_MAX_TRACKS 16
#define TINYUI_LIST_MAX_ITEMS 16
#define TINYUI_GRAPH_MAX_SERIES 8
#define TINYUI_GRAPH_MAX_POINTS 32
#define TINYUI_CANVAS_MAX_COMMANDS 64

typedef struct arm_2d_tile_t arm_2d_tile_t;

struct tinyui_font;
struct tinyui_message_box;
struct kbBtnInfo_t;

typedef void (*tinyui_message_box_callback_t)(struct tinyui_message_box *box, void *user_data);

struct tinyui_app_timer {
    struct tinyui_app *app;
    struct tinyui_app_timer *next;
    unsigned int interval_ms;
    unsigned int next_fire_ticks;
    int repeat;
    int running;
    tinyui_app_timer_cb_t callback;
    void *user_data;
};

void tinyui_app_pump_timers(struct tinyui_app *app, unsigned int now_ticks);

struct tinyui_display_port_state {
    struct tinyui_display_config config;
    tinyui_display_flush_cb_t flush_callback;
    void *flush_user_data;
};

struct tinyui_input_port_state {
    int pointer_x;
    int pointer_y;
    int pointer_pressed;
    enum tinyui_input_key key;
    int key_pressed;
};

struct tinyui_tick_port_state {
    tinyui_tick_get_cb_t callback;
    void *user_data;
};

struct tinyui_os_port_state {
    tinyui_os_lock_cb_t enter;
    tinyui_os_lock_cb_t leave;
    void *lock_user_data;
    tinyui_os_delay_cb_t delay;
    void *delay_user_data;
};

/**
 * @brief Native platform: align to ld grid
 *
 * @param[in] align align
 * @return 0 on success, -1 on failure
 */

int tinyui_native_align_to_ld_grid(enum tinyui_native_align align);

/**
 * @brief Native platform: nav dir to ld
 *
 * @param[in] dir dir
 * @return 0 on success, -1 on failure
 */

int tinyui_native_nav_dir_to_ld(enum tinyui_native_nav_dir dir);

int tinyui_runtime_bridge_bind_ld_event_bridge(struct tinyui_widget *widget,
                                               struct ld_scene_t *scene,
                                               void *sender);
int tinyui_list_set_selected_index_ld(void *widget, int index);
int tinyui_list_get_selected_index_ld(void *widget);
int tinyui_list_sync_selected_index(struct tinyui_list *list, int *selected_index_out);
int tinyui_widget_is_kind(const struct tinyui_widget *widget,
                          enum tinyui_backend_widget_kind kind);
int tinyui_runtime_bridge_unbind_host(struct tinyui_widget *widget);
int tinyui_runtime_bridge_detach_from_parent(struct tinyui_widget *widget);
int tinyui_runtime_bridge_bind_leaf_widget(struct tinyui_widget *widget,
                                            struct tinyui_app *app);
struct tinyui_app *tinyui_widget_owner_app(const struct tinyui_widget *widget);
int tinyui_widget_has_ld_binding(const struct tinyui_widget *widget);
void tinyui_widget_emit_value_changed(tinyui_value_changed_cb cb,
                                      struct tinyui_widget *widget,
                                      int value,
                                      void *user_data);
void tinyui_widget_emit_event(tinyui_event_cb cb,
                              struct tinyui_widget *widget,
                              void *user_data);
void tinyui_widget_emit_clicked(tinyui_event_cb cb,
                                struct tinyui_widget *widget,
                                void *user_data);
void tinyui_widget_sync_ld_value(struct tinyui_widget *widget,
                                 int value);
int tinyui_widget_update_value(struct tinyui_widget *widget,
                               int value,
                               tinyui_value_changed_cb cb,
                               void *user_data);
int tinyui_widget_set_backend_text(struct tinyui_widget *widget, const char *text);
void tinyui_widget_emit_ld_event_bridge(struct tinyui_widget *widget,
                                        enum tinyui_backend_signal signal,
                                        int value);
int tinyui_widget_dispatch_signal(struct tinyui_widget *widget,
                                  enum tinyui_backend_signal signal,
                                  int value,
                                  tinyui_value_changed_cb cb,
                                  void *user_data);
int tinyui_widget_dispatch_event(struct tinyui_widget *widget,
                                 enum tinyui_backend_signal signal,
                                 tinyui_event_cb cb,
                                 void *user_data);
int tinyui_widget_dispatch_native_signal(struct tinyui_widget *widget,
                                         uint32_t native_signal,
                                         uint64_t native_value);

/**
 * @brief Claim focus for a widget
 *
 * @param[in] widget widget
 * @return 0 on success, -1 on failure
 */
int tinyui_widget_claim_backend_focus(struct tinyui_widget *widget);

/**
 * @brief Release focus from a widget
 *
 * @param[in] widget widget
 * @return 0 on success, -1 on failure
 */
int tinyui_widget_release_backend_focus(struct tinyui_widget *widget);

int tinyui_runtime_host_step_app(struct tinyui_app *app);
void tinyui_runtime_host_shutdown_app(struct tinyui_app *app);
int tinyui_window_apply_flex_flow(struct tinyui_window *window, enum tinyui_flex_flow flow);
int tinyui_window_apply_flex_align(struct tinyui_window *window,
                                   enum tinyui_align main_align,
                                   enum tinyui_align cross_align,
                                   enum tinyui_align track_align);
int tinyui_window_apply_flex_gap(struct tinyui_window *window, int item_gap, int track_gap);
int tinyui_window_apply_uniform_padding(struct tinyui_window *window, int padding);
int tinyui_window_apply_explicit_grid_padding(struct tinyui_window *window,
                                              int left,
                                              int top,
                                              int right,
                                              int bottom);
int tinyui_window_apply_grid_columns(struct tinyui_window *window, const int *tracks, int count);
int tinyui_window_apply_grid_rows(struct tinyui_window *window, const int *tracks, int count);
int tinyui_window_apply_grid_gap(struct tinyui_window *window, int row_gap, int col_gap);
int tinyui_window_apply_grid_align(struct tinyui_window *window,
                                   enum tinyui_align col_align,
                                   enum tinyui_align row_align);
int tinyui_theme_apply_widget_style(struct tinyui_widget *widget,
                                    enum tinyui_part part,
                                    enum tinyui_state state,
                                    unsigned int bg_color,
                                    unsigned int text_color,
                                    unsigned int border_color);

enum tinyui_focus_event {
    TINYUI_FOCUS_EVENT_NONE = 0,
    TINYUI_FOCUS_EVENT_ENTER,
    TINYUI_FOCUS_EVENT_LEAVE,
};

enum tinyui_edit_result {
    TINYUI_EDIT_RESULT_NONE = 0,
    TINYUI_EDIT_RESULT_COMMIT,
    TINYUI_EDIT_RESULT_CANCEL,
};

struct tinyui_widget {
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
    const struct tinyui_font *font;
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
    enum tinyui_align grid_x_align;
    enum tinyui_align grid_y_align;
    int has_focus;
    int focus_enter_count;
    int focus_leave_count;
    int focus_change_count;
    enum tinyui_focus_event last_focus_event;
    enum tinyui_edit_result pending_edit_result;
    enum tinyui_edit_result last_edit_result;
    /* ── folded backend fields (Phase C) ──────────────────────────── */
    void *ld_widget;
    uint16_t ld_name_id;
    enum tinyui_backend_widget_kind kind;
    struct tinyui_app *owner;
    struct ld_scene_t *ld_event_bridge_scene;
    void *ld_event_bridge_sender;
    struct tinyui_widget *ld_event_bridge_next;
    int value;
    uint16_t list_item_count;
    enum tinyui_edit_result edit_result_on_finish;
    /* ── B2 运行期生命周期:活宿主注册表链接 ── */
    struct tinyui_widget *reg_prev;
    struct tinyui_widget *reg_next;
    /* ── B2:宿主侧额外清理(仅 keyboard/button 等用,可空)。在 ld depose 前调用,ld 仍存活 ── */
    void (*host_cleanup)(struct tinyui_widget *w);
};

struct tinyui_app {
    struct ld_scene_t  *ld_scene;
    uint16_t            next_ld_name_id;
    void               *runtime_state;
    struct tinyui_theme *theme;
    struct tinyui_window *root_window;
    struct tinyui_widget *focus_owner;
    struct tinyui_widget *editing_owner;
    struct tinyui_app_timer *timers;
    struct tinyui_display_port_state display_port;
    struct tinyui_input_port_state input_port;
    struct tinyui_tick_port_state tick_port;
    struct tinyui_os_port_state os_port;
    /* ── B2:活宿主侵入式双向链表头 ── */
    struct tinyui_widget *host_list_head;
    /* ── P4-4.3: nameId 空闲表（防 uint16 单调回绕） ── */
    uint16_t           *free_name_ids;
    uint16_t            free_name_id_count;
    uint16_t            free_name_id_cap;
};

struct tinyui_theme {
    unsigned int colors[TINYUI_COLOR_COUNT];
    int metrics[TINYUI_METRIC_COUNT];
};

struct tinyui_window {
    struct tinyui_widget widget;
    const char *id;
    int background_offset_x;
    int background_offset_y;
    enum tinyui_flex_flow flex_flow;
    enum tinyui_align flex_main_align;
    enum tinyui_align flex_cross_align;
    enum tinyui_align flex_track_align;
    int flex_item_gap;
    int flex_track_gap;
    int grid_cols[TINYUI_LAYOUT_MAX_TRACKS];
    int grid_rows[TINYUI_LAYOUT_MAX_TRACKS];
    int16_t backend_grid_cols[TINYUI_LAYOUT_MAX_TRACKS];
    int16_t backend_grid_rows[TINYUI_LAYOUT_MAX_TRACKS];
    int grid_col_count;
    int grid_row_count;
    int grid_row_gap;
    int grid_col_gap;
    enum tinyui_align grid_col_align;
    enum tinyui_align grid_row_align;
    /* ── padding fields (Phase C, from tinyui_backend_layout_window_state) */
    int16_t padding_left;
    int16_t padding_top;
    int16_t padding_right;
    int16_t padding_bottom;
    uint8_t has_explicit_flex_padding;
    int16_t grid_padding_left;
    int16_t grid_padding_top;
    int16_t grid_padding_right;
    int16_t grid_padding_bottom;
    uint8_t has_explicit_grid_padding;
    /* ── padding_group storage (ldWindow keeps the pointer alive)
     * Allocated lazily in tinyui_window_sync_padding. */
    void *padding_group_storage;
};

struct tinyui_background {
    struct tinyui_window window;
};

enum tinyui_canvas_command_kind {
    TINYUI_CANVAS_COMMAND_FILL_RECT = 0,
    TINYUI_CANVAS_COMMAND_DRAW_LINE,
    TINYUI_CANVAS_COMMAND_DRAW_IMAGE,
    TINYUI_CANVAS_COMMAND_DRAW_IMAGE_SCALED,
    TINYUI_CANVAS_COMMAND_DRAW_TEXT,
};

struct tinyui_canvas_command {
    enum tinyui_canvas_command_kind kind;
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
    enum tinyui_align align;
    const char *text;
    struct tinyui_image_source *source;
};

struct tinyui_canvas {
    struct tinyui_widget widget;
    const char *id;
    struct tinyui_canvas_command commands[TINYUI_CANVAS_MAX_COMMANDS];
    int command_count;
};

struct tinyui_keyboard_layout_entry {
    char *text;
    unsigned int key_code;
    unsigned int press_color;
    unsigned int release_color;
    int x;
    int y;
    int width;
    int height;
};

struct tinyui_label {
    struct tinyui_widget widget;
    const char *id;
};

struct tinyui_button {
    struct tinyui_widget widget;
    const char *id;
    tinyui_event_cb on_clicked;
    void *user_data;
    tinyui_event_cb on_pressed;
    void *on_pressed_user_data;
    tinyui_event_cb on_released;
    void *on_released_user_data;
    xBtnInfo_t action_info;
};

struct tinyui_keyboard {
    struct tinyui_widget widget;
    const char *id;
    const struct tinyui_keyboard_button *buttons;
    struct tinyui_keyboard_layout_entry *layout_entries;
    void *native_layout;
    int layout_count;
    tinyui_keyboard_event_cb event_cb;
    void *event_user_data;
    tinyui_keyboard_draw_cb draw_cb;
    void *draw_user_data;
    int draw_invocation_count;
    unsigned int last_draw_key_code;
};

struct tinyui_checkbox {
    struct tinyui_widget widget;
    const char *id;
    int checked;
    tinyui_value_changed_cb cb;
    void *user_data;
};

struct tinyui_switch {
    struct tinyui_widget widget;
    const char *id;
    int checked;
    tinyui_value_changed_cb cb;
    void *user_data;
};

struct tinyui_slider {
    struct tinyui_widget widget;
    const char *id;
    int value;
    int min_value;
    int max_value;
    tinyui_value_changed_cb cb;
    void *user_data;
};

struct tinyui_progress_bar {
    struct tinyui_widget widget;
    const char *id;
    int percent;
    int horizontal;
    int inverted;
    unsigned int bg_color;
    unsigned int fg_color;
    unsigned int frame_color;
    int frame_color_size;
    struct tinyui_image_source *bg_source;
    struct tinyui_image_source *fg_source;
    struct tinyui_image_source *frame_source;
};

struct tinyui_animation {
    struct tinyui_widget widget;
    const char *id;
    int width;
    int height;
    int period_ms;
    struct tinyui_image_source *source;
};

struct tinyui_arc {
    struct tinyui_widget widget;
    const char *id;
    float bg_start_angle;
    float bg_end_angle;
    float fg_end_angle;
    float rotation_angle;
    struct tinyui_image_source *quarter_source;
    unsigned int parent_color;
    unsigned int bg_color;
    unsigned int fg_color;
};

struct tinyui_gauge {
    struct tinyui_widget widget;
    const char *id;
    float angle;
    struct tinyui_image_source *bg_source;
    struct tinyui_image_source *pointer_source;
    int centre_offset_x;
    int centre_offset_y;
    unsigned int pointer_color;
    int auto_move;
};

struct tinyui_list_item {
    const char *id;
    const char *text;
};

struct tinyui_icon_slider {
    struct tinyui_widget widget;
    const char *id;
    struct tinyui_list_item items[TINYUI_LIST_MAX_ITEMS];
    struct tinyui_image_source *item_sources[TINYUI_LIST_MAX_ITEMS];
    int item_count;
    int selected_index;
    int horizontal;
    int speed;
    int icon_width;
    int icon_space;
    int columns;
    int rows;
    int pages;
    void (*cb)(struct tinyui_icon_slider *icon_slider, int index, void *user_data);
    void *user_data;
};

struct tinyui_radial_menu {
    struct tinyui_widget widget;
    const char *id;
    struct tinyui_list_item items[TINYUI_LIST_MAX_ITEMS];
    struct tinyui_image_source *item_sources[TINYUI_LIST_MAX_ITEMS];
    int item_count;
    int selected_index;
    int x_axis;
    int y_axis;
    int item_max;
    void (*cb)(struct tinyui_radial_menu *radial_menu, int index, void *user_data);
    void *user_data;
};

struct tinyui_qrcode {
    struct tinyui_widget widget;
    const char *id;
    const char *text;
    unsigned int qr_color;
    unsigned int bg_color;
    int ecc;
    int max_version;
    int zoom;
};

struct tinyui_progress_wheel {
    struct tinyui_widget widget;
    const char *id;
    int percent;
    unsigned int wheel_color;
    unsigned int dot_color;
    int dot_enabled;
};

struct tinyui_date_time {
    struct tinyui_widget widget;
    const char *id;
    const char *format;
    unsigned int text_color;
    unsigned int bg_color;
    enum tinyui_align align;
    int transparent;
    int year;
    int month;
    int day;
    int hour;
    int minute;
    int second;
    int use_system_time;
};

struct tinyui_calendar {
    struct tinyui_widget widget;
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

struct tinyui_clock {
    struct tinyui_widget widget;
    const char *id;
    struct tinyui_image_source *background_source;
    struct tinyui_image_source *hour_pointer_source;
    struct tinyui_image_source *minute_pointer_source;
    struct tinyui_image_source *second_pointer_source;
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

struct tinyui_list {
    struct tinyui_widget widget;
    const char *id;
    struct tinyui_list_item items[TINYUI_LIST_MAX_ITEMS];
    const char *backend_item_ids[TINYUI_LIST_MAX_ITEMS];
    const unsigned char *backend_item_texts[TINYUI_LIST_MAX_ITEMS];
    int item_count;
    int selected_index;
    void (*cb)(struct tinyui_list *list, int index, void *user_data);
    void *user_data;
};

struct tinyui_message_box {
    struct tinyui_widget widget;
    const char *id;
    const char *title;
    const char *message;
    const char *confirm_text;
    const char *buttons[TINYUI_LIST_MAX_ITEMS];
    int button_count;
    unsigned int title_color;
    unsigned int message_color;
    unsigned int button_color;
    unsigned int release_color;
    unsigned int press_color;
    unsigned int bg_color;
    tinyui_message_box_callback_t on_confirm;
    void *on_confirm_user_data;
    tinyui_message_box_indexed_callback_t on_confirm_indexed;
    void *on_confirm_indexed_user_data;
};

struct tinyui_text {
    struct tinyui_widget widget;
    const char *id;
};

struct tinyui_line_edit {
    struct tinyui_widget widget;
    const char *id;
    enum tinyui_align align;
    enum tinyui_line_edit_type type;
    unsigned int keyboard_binding;
    int editing;
    tinyui_line_edit_finished_cb on_edit_finished;
    void *on_edit_finished_user_data;
};

struct tinyui_combo_box {
    struct tinyui_widget widget;
    const char *id;
    void *_host_backend; /* Native dropdown host object owned by ldComboBox. */
    struct tinyui_list_item items[TINYUI_LIST_MAX_ITEMS];
    const char *backend_item_ids[TINYUI_LIST_MAX_ITEMS];
    const unsigned char *backend_item_texts[TINYUI_LIST_MAX_ITEMS];
    int item_count;
    int item_max;
    int selected_index;
    struct tinyui_image_source *dropdown_source;
    void (*cb)(struct tinyui_combo_box *combo_box, int index, void *user_data);
    void *user_data;
};

struct tinyui_scroll_selecter {
    struct tinyui_widget widget;
    const char *id;
    void *_host_backend; /* Native picker host object owned by ldScrollSelecter. */
    struct tinyui_list_item items[TINYUI_LIST_MAX_ITEMS];
    const char *backend_item_ids[TINYUI_LIST_MAX_ITEMS];
    const unsigned char *backend_item_texts[TINYUI_LIST_MAX_ITEMS];
    int item_count;
    int selected_index;
    int edit_mode;
    int transparent;
    int speed;
    struct tinyui_image_source *bg_source;
    struct tinyui_image_source *indicator_source;
};

struct tinyui_image {
    struct tinyui_widget widget;
    const char *id;
    struct tinyui_image_source *source;
};

struct tinyui_graph {
    struct tinyui_widget widget;
    const char *id;
    int series_max;
    int series_count;
    int x_axis;
    int y_axis;
    int axis_offset;
    int frame_space;
    int grid_offset;
    struct tinyui_image_source *point_mask_source;
    int series_point_counts[TINYUI_GRAPH_MAX_SERIES];
};

struct tinyui_table {
    struct tinyui_widget widget;
    const char *id;
    unsigned int keyboard_binding;
    int row_count;
    int column_count;
    int current_row;
    int current_column;
};

/* ── Phase C1 core helper declarations ────────────────────────────────────── */

/**
 * @brief Generic leaf widget factory (C1-T4 name-locked).
 *
 * Allocates @p host_size bytes for the host object (whose first member must be
 * struct tinyui_widget), calls @p ld_init_cb to create the backing ld widget,
 * attaches it to the ld tree under @p parent, and binds pInfo.
 *
 * @param[in] parent      Parent widget (must not be NULL)
 * @param[in] kind        Backend widget kind for the new leaf
 * @param[in] ld_init_cb  Callback that allocates and initialises the ld widget.
 *                        Receives @p ctx plus the scene, name_id and
 *                        parent_name_id; must return the new ld widget pointer
 *                        (NULL on failure).
 * @param[in] ctx         Opaque context forwarded to @p ld_init_cb
 * @param[in] host_size   sizeof of the concrete host struct (>= sizeof(struct
 *                        tinyui_widget))
 * @return Pointer to the embedded struct tinyui_widget on success, NULL on failure
 */
void tinyui_app_register_host(struct tinyui_app *app, struct tinyui_widget *w);
void tinyui_app_unregister_host(struct tinyui_app *app, struct tinyui_widget *w);
struct tinyui_widget *tinyui_app_lookup_host(const struct tinyui_app *app, uint16_t name_id);
uint16_t tinyui_app_alloc_name_id(struct tinyui_app *app);
void tinyui_app_free_name_id(struct tinyui_app *app, uint16_t id);
struct tinyui_widget *tinyui_widget_from_ld(const void *ld_node);
struct tinyui_widget *tinyui_widget_from_ld_scene(const struct ld_scene_t *scene, const void *ld_node);

struct tinyui_widget *tinyui_widget_create_leaf(
    struct tinyui_widget *parent,
    enum tinyui_backend_widget_kind kind,
    void *(*ld_init_cb)(void *ctx, struct ld_scene_t *scene,
                        uint16_t name_id, uint16_t parent_name_id),
    void *ctx,
    size_t host_size);

/**
 * @brief Detach widget from parent in the ld tree
 */
int tinyui_widget_detach_from_parent(struct tinyui_widget *w);

/**
 * @brief Common destroy: host_cleanup + depose ld via ptGuiFunc + free host
 */
void tinyui_widget_destroy_common(struct tinyui_widget *w);

/**
 * @brief Convert RGB888 packed value to ldColor (RGB565 via __RGB macro)
 *        Return type widens to unsigned int to avoid pulling arm_2d types here.
 */
unsigned int tinyui_rgb_to_ld_color(unsigned int rgb888);

/**
 * @brief Convert ldColor (RGB565) back to approximately RGB888
 */
unsigned int tinyui_ld_color_to_rgb(unsigned int color);

/**
 * @brief Map tinyui_align to arm_2d_align_t (horizontal axis, START/CENTER/END)
 *        Return is int compatible with arm_2d_align_t enum.
 */
int tinyui_align_to_arm2d(enum tinyui_align align);

/**
 * @brief Claim input focus
 *
 * @param[in] widget Widget instance
 * @return 0 on success, -1 on failure
 */

int tinyui_widget_claim_focus(struct tinyui_widget *widget);

/**
 * @brief Widget: release focus
 *
 * @param[in] widget Widget instance
 * @return 0 on success, -1 on failure
 */

int tinyui_widget_release_focus(struct tinyui_widget *widget);

/**
 * @brief Widget: is focus owner
 *
 * @param[in] widget Widget instance
 * @return 0 on success, -1 on failure
 */

int tinyui_widget_is_focus_owner(const struct tinyui_widget *widget);

/**
 * @brief Widget: mark edit result
 *
 * @param[in] widget Widget instance
 * @param[in] result result
 * @return 0 on success, -1 on failure
 */

int tinyui_widget_mark_edit_result(struct tinyui_widget *widget, enum tinyui_edit_result result);

/**
 * @brief Widget: claim editing
 *
 * @param[in] widget Widget instance
 * @return 0 on success, -1 on failure
 */

int tinyui_widget_claim_editing(struct tinyui_widget *widget);

/**
 * @brief Widget: release editing
 *
 * @param[in] widget Widget instance
 * @return 0 on success, -1 on failure
 */

int tinyui_widget_release_editing(struct tinyui_widget *widget);

/**
 * @brief Widget: is editing owner
 *
 * @param[in] widget Widget instance
 * @return 0 on success, -1 on failure
 */

int tinyui_widget_is_editing_owner(const struct tinyui_widget *widget);

/**
 * @brief Get the folded core widget stored in a window host object.
 *
 * This is a compatibility accessor for tests and internal seams that need the
 * embedded struct tinyui_widget.
 *
 * @param[in] window  Window instance (may be NULL)
 * @return Pointer to the embedded struct tinyui_widget, or NULL
 */
void *tinyui_window_get_backend_widget(struct tinyui_window *window);

#endif
