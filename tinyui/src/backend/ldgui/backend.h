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

#ifndef PICOUI_BACKEND_LDGUI_H
#define PICOUI_BACKEND_LDGUI_H

#include <stdint.h>

#include "image.h"
#include "native.h"
#include "widget.h"
#include "window.h"

struct picoui_widget;
struct picoui_message_box;
struct picoui_app;
struct picoui_theme;
struct picoui_window;
struct picoui_animation;
struct picoui_checkbox;
struct picoui_switch;
struct picoui_progress_wheel;
struct picoui_date_time;
struct picoui_clock;
struct picoui_arc;
struct picoui_gauge;
struct picoui_list;
struct picoui_combo_box;
struct picoui_icon_slider;
struct picoui_radial_menu;
struct picoui_scroll_selecter;
struct picoui_table;
struct picoui_progress_wheel_props;
struct picoui_calendar;
struct picoui_canvas;
struct ld_scene_t;
enum picoui_line_edit_type;

#define PICOUI_BACKEND_LAYOUT_MAX_TRACKS 16
#define PICOUI_BACKEND_LIST_MAX_ITEMS 16

enum picoui_backend_widget_kind {
    PICOUI_BACKEND_WIDGET_WINDOW = 0,
    PICOUI_BACKEND_WIDGET_BACKGROUND,
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
    PICOUI_BACKEND_WIDGET_ANIMATION,
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
    PICOUI_BACKEND_WIDGET_CANVAS,
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
    int16_t flex_flow;
    int16_t flex_main_align;
    int16_t flex_cross_align;
    int16_t flex_track_align;
    int16_t padding;
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
    int16_t flex_item_gap;
    int16_t flex_track_gap;
    int16_t grid_cols[PICOUI_BACKEND_LAYOUT_MAX_TRACKS];
    int16_t grid_rows[PICOUI_BACKEND_LAYOUT_MAX_TRACKS];
    int16_t grid_col_count;
    int16_t grid_row_count;
    int16_t grid_row_gap;
    int16_t grid_col_gap;
    int16_t grid_col_align;
    int16_t grid_row_align;
};

struct picoui_backend_layout_child_state {
    int16_t flex_grow;
    uint8_t flex_new_track;
    uint8_t ignore_layout;
    int16_t grid_col;
    int16_t grid_row;
    int16_t grid_col_span;
    int16_t grid_row_span;
    int16_t grid_x_align;
    int16_t grid_y_align;
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
    int16_t last_signal;
    uint16_t reserved_signal_padding;
    uint32_t last_native_signal;
    uint64_t last_native_value;
    int dispatch_count;
    struct picoui_theme *theme;
    void *ld_widget;
    uint16_t ld_name_id;
    const char *list_item_ids[PICOUI_BACKEND_LIST_MAX_ITEMS];
    uint16_t list_item_count;
    uint16_t reserved_list_padding;
    unsigned int data_model_identity;
    unsigned int data_model_epoch;
    int16_t data_truth_policy;
    int16_t last_data_source;
    int16_t edit_result_on_finish;
    uint16_t reserved_edit_padding;
    struct picoui_backend_layout_window_state window_layout;
    struct picoui_backend_layout_child_state child_layout;
    uint16_t runtime_evidence_flags;
    uint16_t open;
};

struct picoui_backend_app_state {
    struct picoui_theme *theme;
    struct ld_scene_t *ld_scene;
    uint16_t next_ld_name_id;
    void *runtime_state;
    int last_window_switch_mode;
    unsigned int last_window_switch_duration_ms;
};

struct picoui_progress_wheel_test_dispose_snapshot;
struct picoui_table_test_dispose_snapshot;

/**
 * @brief Initialize app backend
 *
 * @param[in] app Application instance
 * @return 0 on success, -1 on failure
 */

int picoui_backend_app_init(struct picoui_app *app);

/**
 * @brief Run app backend
 *
 * @param[in] app Application instance
 * @param[in] window Window instance
 * @return 0 on success, -1 on failure
 */

int picoui_backend_app_run(struct picoui_app *app, struct picoui_window *window);

/**
 * @brief Shutdown app backend
 *
 * @param[in] app Application instance
 */

void picoui_backend_app_shutdown(struct picoui_app *app);

/**
 * @brief Set background angle of arc backend
 *
 * @param[in] arc Arc widget instance
 * @param[in] bg_start_angle Background arc start angle
 * @param[in] bg_end_angle Background arc end angle
 * @return 0 on success, -1 on failure
 */

int picoui_backend_arc_set_background_angle(struct picoui_arc *arc, float bg_start_angle, float bg_end_angle);

/**
 * @brief Set foreground angle of arc backend
 *
 * @param[in] arc Arc widget instance
 * @param[in] fg_end_angle Foreground arc end angle
 * @return 0 on success, -1 on failure
 */

int picoui_backend_arc_set_foreground_angle(struct picoui_arc *arc, float fg_end_angle);

/**
 * @brief Set rotation angle of arc backend
 *
 * @param[in] arc Arc widget instance
 * @param[in] rotation_angle Arc rotation angle
 * @return 0 on success, -1 on failure
 */

int picoui_backend_arc_set_rotation_angle(struct picoui_arc *arc, float rotation_angle);

/**
 * @brief Set color of arc backend
 *
 * @param[in] arc Arc widget instance
 * @param[in] bg_color Background color
 * @param[in] fg_color Foreground color
 * @return 0 on success, -1 on failure
 */

int picoui_backend_arc_set_color(struct picoui_arc *arc, unsigned int bg_color, unsigned int fg_color);

/**
 * @brief Get background angle from arc backend
 *
 * @param[out] arc Arc widget instance
 * @param[in] bg_start_angle Background arc start angle
 * @param[in] bg_angle bg angle
 * @return The property value, negative on error
 */

int picoui_backend_arc_get_background_angle(struct picoui_arc *arc, float *bg_start_angle, float *bg_angle);

/**
 * @brief Get foreground angle from arc backend
 *
 * @param[out] arc Arc widget instance
 * @param[in] fg_end_angle Foreground arc end angle
 * @return The property value, negative on error
 */

int picoui_backend_arc_get_foreground_angle(struct picoui_arc *arc, float *fg_end_angle);

/**
 * @brief Get rotation angle from arc backend
 *
 * @param[out] arc Arc widget instance
 * @param[in] rotation_angle Arc rotation angle
 * @return The property value, negative on error
 */

int picoui_backend_arc_get_rotation_angle(struct picoui_arc *arc, float *rotation_angle);

/**
 * @brief Get color from arc backend
 *
 * @param[out] arc Arc widget instance
 * @param[in] bg_color Background color
 * @param[in] fg_color Foreground color
 * @return The property value, negative on error
 */

int picoui_backend_arc_get_color(struct picoui_arc *arc, unsigned int *bg_color, unsigned int *fg_color);

/**
 * @brief Set quarter source of arc backend
 *
 * @param[in] arc Arc widget instance
 * @param[in] source Image source
 * @return 0 on success, -1 on failure
 */

int picoui_backend_arc_set_quarter_source(struct picoui_arc *arc, struct picoui_image_source *source);

/**
 * @brief Set parent color of arc backend
 *
 * @param[in] arc Arc widget instance
 * @param[in] parent_color parent color
 * @return 0 on success, -1 on failure
 */

int picoui_backend_arc_set_parent_color(struct picoui_arc *arc, unsigned int parent_color);

/**
 * @brief Create backend for gauge
 *
 * @param[in] parent Parent widget
 * @param[in] id Widget identifier string
 */

/**
 * @brief Set angle of gauge backend
 *
 * @param[in] gauge Gauge widget instance
 * @param[in] angle Angle in degrees
 * @return 0 on success, -1 on failure
 */

int picoui_backend_gauge_set_angle(struct picoui_gauge *gauge, float angle);

/**
 * @brief Get angle from gauge backend
 *
 * @param[out] gauge Gauge widget instance
 * @param[in] angle Angle in degrees
 * @return The property value, negative on error
 */

int picoui_backend_gauge_get_angle(struct picoui_gauge *gauge, float *angle);

/**
 * @brief Set bg source of gauge backend
 *
 * @param[in] gauge Gauge widget instance
 * @param[in] source Image source
 * @return 0 on success, -1 on failure
 */

int picoui_backend_gauge_set_bg_source(struct picoui_gauge *gauge, struct picoui_image_source *source);

/**
 * @brief Set pointer source of gauge backend
 *
 * @param[in] gauge Gauge widget instance
 * @param[in] source Image source
 * @return 0 on success, -1 on failure
 */

int picoui_backend_gauge_set_pointer_source(struct picoui_gauge *gauge, struct picoui_image_source *source);

/**
 * @brief Set centre offset of gauge backend
 *
 * @param[in] gauge Gauge widget instance
 * @param[in] centre_offset_x centre offset x
 * @param[in] centre_offset_y centre offset y
 * @return 0 on success, -1 on failure
 */

int picoui_backend_gauge_set_centre_offset(struct picoui_gauge *gauge, int centre_offset_x, int centre_offset_y);

/**
 * @brief Set trail of gauge backend
 *
 * @param[in] gauge Gauge widget instance
 * @param[in] bg_trail_source bg trail source
 * @param[in] pointer_trail_source pointer trail source
 * @return 0 on success, -1 on failure
 */

int picoui_backend_gauge_set_trail(struct picoui_gauge *gauge,
                                   struct picoui_image_source *bg_trail_source,
                                   struct picoui_image_source *pointer_trail_source);

/**
 * @brief Set progress bar of gauge backend
 *
 * @param[in] gauge Gauge widget instance
 * @param[in] bg_progress_source bg progress source
 * @param[in] pointer_progress_source pointer progress source
 * @return 0 on success, -1 on failure
 */

int picoui_backend_gauge_set_progress_bar(struct picoui_gauge *gauge,
                                          struct picoui_image_source *bg_progress_source,
                                          struct picoui_image_source *pointer_progress_source);

/**
 * @brief Set pointer color of gauge backend
 *
 * @param[in] gauge Gauge widget instance
 * @param[in] pointer_color pointer color
 * @return 0 on success, -1 on failure
 */

int picoui_backend_gauge_set_pointer_color(struct picoui_gauge *gauge, unsigned int pointer_color);

/**
 * @brief Get pointer color from gauge backend
 *
 * @param[out] gauge Gauge widget instance
 * @param[in] pointer_color pointer color
 * @return The property value, negative on error
 */

int picoui_backend_gauge_get_pointer_color(struct picoui_gauge *gauge, unsigned int *pointer_color);

/**
 * @brief Set auto move of gauge backend
 *
 * @param[in] gauge Gauge widget instance
 * @param[in] auto_move auto move
 * @return 0 on success, -1 on failure
 */

int picoui_backend_gauge_set_auto_move(struct picoui_gauge *gauge, int auto_move);

/**
 * @brief Get auto move from gauge backend
 *
 * @param[out] gauge Gauge widget instance
 * @param[in] auto_move auto move
 * @return The property value, negative on error
 */

int picoui_backend_gauge_get_auto_move(struct picoui_gauge *gauge, int *auto_move);

/**
 * @brief Create backend for progress bar
 *
 * @param[in] parent Parent widget
 * @param[in] id Widget identifier string
 */

/**
 * @brief Create backend for animation
 *
 * @param[in] parent Parent widget
 * @param[in] id Widget identifier string
 * @param[in] width Width in pixels
 * @param[in] height Height in pixels
 * @param[in] source Image source
 * @param[in] period_ms Period in milliseconds
 */

/**
 * @brief Create backend for qrcode
 *
 * @param[in] parent Parent widget
 * @param[in] id Widget identifier string
 */

/**
 * @brief Set qr color of qrcode backend
 *
 * @param[in] backend_widget backend widget
 * @param[in] rgb RGB color value (0xRRGGBB)
 * @return 0 on success, -1 on failure
 */

int picoui_backend_qrcode_set_qr_color(void *backend_widget, unsigned int rgb);

/**
 * @brief Set bg color of qrcode backend
 *
 * @param[in] backend_widget backend widget
 * @param[in] rgb RGB color value (0xRRGGBB)
 * @return 0 on success, -1 on failure
 */

int picoui_backend_qrcode_set_bg_color(void *backend_widget, unsigned int rgb);

/**
 * @brief Set ecc of qrcode backend
 *
 * @param[in] backend_widget backend widget
 * @param[in] ecc ecc
 * @return 0 on success, -1 on failure
 */

int picoui_backend_qrcode_set_ecc(void *backend_widget, int ecc);

/**
 * @brief Set max version of qrcode backend
 *
 * @param[in] backend_widget backend widget
 * @param[in] max_version max version
 * @return 0 on success, -1 on failure
 */

int picoui_backend_qrcode_set_max_version(void *backend_widget, int max_version);

/**
 * @brief Set zoom of qrcode backend
 *
 * @param[in] backend_widget backend widget
 * @param[in] zoom zoom
 * @return 0 on success, -1 on failure
 */

int picoui_backend_qrcode_set_zoom(void *backend_widget, int zoom);

void picoui_backend_progress_wheel_test_reset_state(void);
void picoui_backend_progress_wheel_test_fail_next_set_percent(void);
int picoui_backend_progress_wheel_test_take_last_dispose_snapshot(
    struct picoui_progress_wheel_test_dispose_snapshot *snapshot);
void picoui_backend_progress_wheel_test_capture_dispose_snapshot(
    struct picoui_backend_widget *backend,
    int detach_result,
    int unbind_result);
void picoui_backend_table_test_fail_next_set_keyboard_binding(void);
void picoui_backend_table_test_reset_state(void);
int picoui_backend_table_test_take_last_dispose_snapshot(
    struct picoui_table_test_dispose_snapshot *snapshot);

/**
 * @brief Set background offset of window backend
 *
 * @param[in] window Window instance
 * @param[in] offset_x Horizontal offset
 * @param[in] offset_y Vertical offset
 * @return 0 on success, -1 on failure
 */

int picoui_backend_window_set_background_offset(struct picoui_window *window,
                                                int offset_x,
                                                int offset_y);

/**
 * @brief Create backend for message box
 *
 * @param[in] parent Parent widget
 * @param[in] id Widget identifier string
 */

/**
 * @brief message: box set title
 *
 * @param[in] box box
 * @param[in] title title
 * @return 0 on success, -1 on failure
 */

/**
 * @brief Create backend for date time
 *
 * @param[in] parent Parent widget
 * @param[in] id Widget identifier string
 */

/**
 * @brief Create backend for text
 *
 * @param[in] parent Parent widget
 * @param[in] id Widget identifier string
 */

/**
 * @brief Create backend for keyboard
 *
 * @param[in] parent Parent widget
 * @param[in] id Widget identifier string
 */

/**
 * @brief Create backend for graph
 *
 * @param[in] parent Parent widget
 * @param[in] id Widget identifier string
 * @param[in] series_max series max
 */

/**
 * @brief Create backend for canvas
 *
 * @param[in] parent Parent widget
 * @param[in] id Widget identifier string
 */

/**
 * @brief progress: bar set bg source
 *
 * @param[in] backend_widget backend widget
 * @param[in] source Image source
 * @return 0 on success, -1 on failure
 */

int picoui_backend_progress_bar_set_bg_source(void *backend_widget, struct picoui_image_source *source);

/**
 * @brief progress: bar set fg source
 *
 * @param[in] backend_widget backend widget
 * @param[in] source Image source
 * @return 0 on success, -1 on failure
 */

int picoui_backend_progress_bar_set_fg_source(void *backend_widget, struct picoui_image_source *source);

/**
 * @brief progress: bar set frame source
 *
 * @param[in] backend_widget backend widget
 * @param[in] source Image source
 * @return 0 on success, -1 on failure
 */

int picoui_backend_progress_bar_set_frame_source(void *backend_widget, struct picoui_image_source *source);

/**
 * @brief progress: bar set color
 *
 * @param[in] backend_widget backend widget
 * @param[in] bg_color Background color
 * @param[in] fg_color Foreground color
 * @return 0 on success, -1 on failure
 */

int picoui_backend_progress_bar_set_color(void *backend_widget,
                                          unsigned int bg_color,
                                          unsigned int fg_color);

/**
 * @brief progress: bar set frame color
 *
 * @param[in] backend_widget backend widget
 * @param[in] frame_color frame color
 * @param[in] frame_color_size frame color size
 * @return 0 on success, -1 on failure
 */

int picoui_backend_progress_bar_set_frame_color(void *backend_widget,
                                                unsigned int frame_color,
                                                int frame_color_size);

/**
 * @brief progress: bar set inverted
 *
 * @param[in] backend_widget backend widget
 * @param[in] inverted inverted
 * @return 0 on success, -1 on failure
 */

int picoui_backend_progress_bar_set_inverted(void *backend_widget, int inverted);

/**
 * @brief progress: bar get inverted
 *
 * @param[in] backend_widget backend widget
 * @return The property value, negative on error
 */

int picoui_backend_progress_bar_get_inverted(void *backend_widget);

/**
 * @brief message: box set on confirm
 *
 * @param[in] box box
 * @return 0 on success, -1 on failure
 */

/**
 * @brief set: text
 *
 * @param[in] backend_widget backend widget
 * @param[in] text Text widget instance
 * @return 0 on success, -1 on failure
 */

int tinyui_widget_set_backend_text(void *backend_widget, const char *text);

/**
 * @brief line: edit set text
 *
 * @param[in] backend_widget backend widget
 * @param[in] text Text widget instance
 * @return 0 on success, -1 on failure
 */

int picoui_backend_line_edit_set_text(void *backend_widget, const char *text);

/**
 * @brief line: edit set align
 *
 * @param[in] backend_widget backend widget
 * @param[in] align align
 * @return 0 on success, -1 on failure
 */

int picoui_backend_line_edit_set_align(void *backend_widget, enum picoui_align align);

/**
 * @brief line: edit set color
 *
 * @param[in] backend_widget backend widget
 * @param[in] text_color Text color
 * @param[in] background_color background color
 * @param[in] frame_color frame color
 * @return 0 on success, -1 on failure
 */

int picoui_backend_line_edit_set_color(void *backend_widget,
                                       unsigned int text_color,
                                       unsigned int background_color,
                                       unsigned int frame_color);

/**
 * @brief line: edit get text
 *
 * @param[in] backend_widget backend widget
 */

const char *picoui_backend_line_edit_get_text(void *backend_widget);

/**
 * @brief line: edit set type
 *
 * @param[in] backend_widget backend widget
 * @param[in] type Type
 * @return 0 on success, -1 on failure
 */

int picoui_backend_line_edit_set_type(void *backend_widget, enum picoui_line_edit_type type);

/**
 * @brief line: edit get type
 *
 * @param[in] backend_widget backend widget
 * @param[out] type Type
 * @return The property value, negative on error
 */

int picoui_backend_line_edit_get_type(void *backend_widget, enum picoui_line_edit_type *type);

/**
 * @brief line: edit set keyboard
 *
 * @param[in] backend_widget backend widget
 * @param[in] keyboard_binding keyboard binding
 * @return 0 on success, -1 on failure
 */

int picoui_backend_line_edit_set_keyboard(void *backend_widget, unsigned int keyboard_binding);

/**
 * @brief line: edit set keyboard binding
 *
 * @param[in] backend_widget backend widget
 * @param[in] keyboard_binding keyboard binding
 * @return 0 on success, -1 on failure
 */

int picoui_backend_line_edit_set_keyboard_binding(void *backend_widget,
                                                  unsigned int keyboard_binding);

/**
 * @brief line: edit get keyboard binding
 *
 * @param[in] backend_widget backend widget
 * @param[in] keyboard_binding keyboard binding
 * @return The property value, negative on error
 */

int picoui_backend_line_edit_get_keyboard_binding(void *backend_widget,
                                                  unsigned int *keyboard_binding);

/**
 * @brief line: edit bind host
 *
 * @param[in] backend_widget backend widget
 * @return 0 on success, -1 on failure
 */

int picoui_backend_line_edit_bind_host(void *backend_widget);

/**
 * @brief line: edit get editing
 *
 * @param[in] backend_widget backend widget
 * @param[in] editing editing
 * @return The property value, negative on error
 */

int picoui_backend_line_edit_get_editing(void *backend_widget, int *editing);

/**
 * @brief combo: box set items
 *
 * @param[in] backend_widget backend widget
 * @param[in] item_ids item ids
 * @param[in] items items
 * @param[in] item_count item count
 * @return 0 on success, -1 on failure
 */

int picoui_backend_combo_box_set_items(void *backend_widget,
                                       const char *const *item_ids,
                                       const unsigned char *const *items,
                                       int item_count);

/**
 * @brief combo: box set text color
 *
 * @param[in] backend_widget backend widget
 * @param[in] rgb RGB color value (0xRRGGBB)
 * @return 0 on success, -1 on failure
 */

int picoui_backend_combo_box_set_text_color(void *backend_widget, unsigned int rgb);

/**
 * @brief combo: box set bg color
 *
 * @param[in] backend_widget backend widget
 * @param[in] rgb RGB color value (0xRRGGBB)
 * @return 0 on success, -1 on failure
 */

int picoui_backend_combo_box_set_bg_color(void *backend_widget, unsigned int rgb);

/**
 * @brief combo: box set frame color
 *
 * @param[in] backend_widget backend widget
 * @param[in] rgb RGB color value (0xRRGGBB)
 * @return 0 on success, -1 on failure
 */

int picoui_backend_combo_box_set_frame_color(void *backend_widget, unsigned int rgb);

/**
 * @brief combo: box set select color
 *
 * @param[in] backend_widget backend widget
 * @param[in] rgb RGB color value (0xRRGGBB)
 * @return 0 on success, -1 on failure
 */

int picoui_backend_combo_box_set_select_color(void *backend_widget, unsigned int rgb);

/**
 * @brief combo: box set item max
 *
 * @param[in] backend_widget backend widget
 * @param[in] item_max item max
 * @return 0 on success, -1 on failure
 */

int picoui_backend_combo_box_set_item_max(void *backend_widget, int item_max);

/**
 * @brief combo: box set dropdown source
 *
 * @param[in] backend_widget backend widget
 * @param[in] source Image source
 * @return 0 on success, -1 on failure
 */

int picoui_backend_combo_box_set_dropdown_source(void *backend_widget,
                                                 struct picoui_image_source *source);

/**
 * @brief combo: box set selected index
 *
 * @param[in] backend_widget backend widget
 * @param[in] index Index
 * @return 0 on success, -1 on failure
 */

int picoui_backend_combo_box_set_selected_index(void *backend_widget, int index);

/**
 * @brief combo: box get selected index
 *
 * @param[in] backend_widget backend widget
 * @return The property value, negative on error
 */

int picoui_backend_combo_box_get_selected_index(void *backend_widget);

/**
 * @brief combo: box get text
 *
 * @param[in] backend_widget backend widget
 * @param[in] index Index
 */

const char *picoui_backend_combo_box_get_text(void *backend_widget, int index);

/**
 * @brief combo: box sync selected index
 *
 * @param[in] combo_box Combo box widget instance
 * @param[in] selected_index_out selected index out
 * @return 0 on success, -1 on failure
 */

int picoui_backend_combo_box_sync_selected_index(struct picoui_combo_box *combo_box,
                                                 int *selected_index_out);

/**
 * @brief combo: box bind host
 *
 * @param[in] backend_widget backend widget
 * @return 0 on success, -1 on failure
 */

int picoui_backend_combo_box_bind_host(void *backend_widget);

/**
 * @brief combo: box get open
 *
 * @param[in] backend_widget backend widget
 * @param[in] is_open is open
 * @return The property value, negative on error
 */

int picoui_backend_combo_box_get_open(void *backend_widget, int *is_open);

/**
 * @brief scroll: selecter set items
 *
 * @param[in] backend_widget backend widget
 * @param[in] item_ids item ids
 * @param[in] items items
 * @param[in] item_count item count
 * @return 0 on success, -1 on failure
 */

int picoui_backend_scroll_selecter_set_items(void *backend_widget,
                                             const char *const *item_ids,
                                             const unsigned char *const *items,
                                             int item_count);

/**
 * @brief scroll: selecter set text color
 *
 * @param[in] backend_widget backend widget
 * @param[in] rgb RGB color value (0xRRGGBB)
 * @return 0 on success, -1 on failure
 */

int picoui_backend_scroll_selecter_set_text_color(void *backend_widget, unsigned int rgb);

/**
 * @brief scroll: selecter set bg color
 *
 * @param[in] backend_widget backend widget
 * @param[in] rgb RGB color value (0xRRGGBB)
 * @return 0 on success, -1 on failure
 */

int picoui_backend_scroll_selecter_set_bg_color(void *backend_widget, unsigned int rgb);

/**
 * @brief scroll: selecter set indicator color
 *
 * @param[in] backend_widget backend widget
 * @param[in] rgb RGB color value (0xRRGGBB)
 * @return 0 on success, -1 on failure
 */

int picoui_backend_scroll_selecter_set_indicator_color(void *backend_widget, unsigned int rgb);

/**
 * @brief scroll: selecter set bg source
 *
 * @param[in] backend_widget backend widget
 * @param[in] source Image source
 * @return 0 on success, -1 on failure
 */

int picoui_backend_scroll_selecter_set_bg_source(void *backend_widget,
                                                 struct picoui_image_source *source);

/**
 * @brief scroll: selecter set indicator source
 *
 * @param[in] backend_widget backend widget
 * @param[in] source Image source
 * @return 0 on success, -1 on failure
 */

int picoui_backend_scroll_selecter_set_indicator_source(void *backend_widget,
                                                        struct picoui_image_source *source);

/**
 * @brief scroll: selecter set transparent
 *
 * @param[in] backend_widget backend widget
 * @param[in] transparent transparent
 * @return 0 on success, -1 on failure
 */

int picoui_backend_scroll_selecter_set_transparent(void *backend_widget, int transparent);

/**
 * @brief scroll: selecter set speed
 *
 * @param[in] backend_widget backend widget
 * @param[in] speed speed
 * @return 0 on success, -1 on failure
 */

int picoui_backend_scroll_selecter_set_speed(void *backend_widget, int speed);

/**
 * @brief scroll: selecter set select text
 *
 * @param[in] backend_widget backend widget
 * @param[in] text Text widget instance
 * @return 0 on success, -1 on failure
 */

int picoui_backend_scroll_selecter_set_select_text(void *backend_widget, const char *text);

/**
 * @brief scroll: selecter set selected index
 *
 * @param[in] backend_widget backend widget
 * @param[in] index Index
 * @return 0 on success, -1 on failure
 */

int picoui_backend_scroll_selecter_set_selected_index(void *backend_widget, int index);

/**
 * @brief scroll: selecter get selected index
 *
 * @param[in] backend_widget backend widget
 * @return The property value, negative on error
 */

int picoui_backend_scroll_selecter_get_selected_index(void *backend_widget);

/**
 * @brief scroll: selecter get selected text
 *
 * @param[in] backend_widget backend widget
 */

const char *picoui_backend_scroll_selecter_get_selected_text(void *backend_widget);

/**
 * @brief scroll: selecter sync selected index
 *
 * @param[in] scroll_selecter Scroll selecter widget instance
 * @param[in] selected_index_out selected index out
 * @return 0 on success, -1 on failure
 */

int picoui_backend_scroll_selecter_sync_selected_index(struct picoui_scroll_selecter *scroll_selecter,
                                                       int *selected_index_out);

/**
 * @brief scroll: selecter set edit mode
 *
 * @param[in] backend_widget backend widget
 * @param[in] is_edit is edit
 * @return 0 on success, -1 on failure
 */

int picoui_backend_scroll_selecter_set_edit_mode(void *backend_widget, int is_edit);

/**
 * @brief scroll: selecter get edit mode
 *
 * @param[in] backend_widget backend widget
 * @param[in] is_edit is edit
 * @return The property value, negative on error
 */

int picoui_backend_scroll_selecter_get_edit_mode(void *backend_widget, int *is_edit);

/**
 * @brief Set cell text of table backend
 *
 * @param[in] backend_widget backend widget
 * @param[in] row Row index
 * @param[in] column Column index
 * @param[in] text Text widget instance
 * @return 0 on success, -1 on failure
 */

/**
 * @brief Set item image of table backend
 *
 * @param[in] backend_widget backend widget
 * @param[in] row Row index
 * @param[in] column Column index
 * @param[in] x X coordinate
 * @param[in] y Y coordinate
 * @param[in] source Image source
 * @param[in] mask_color mask color
 * @return 0 on success, -1 on failure
 */

/**
 * @brief Set item align of table backend
 *
 * @param[in] backend_widget backend widget
 * @param[in] row Row index
 * @param[in] column Column index
 * @param[in] align align
 * @return 0 on success, -1 on failure
 */

/**
 * @brief table: navigate
 *
 * @param[in] backend_widget backend widget
 * @param[in] dir dir
 * @return 0 on success, -1 on failure
 */

/**
 * @brief Get item region from table backend
 *
 * @param[in] backend_widget backend widget
 * @param[in] row Row index
 * @param[in] column Column index
 * @param[in] region_out region out
 * @return The property value, negative on error
 */

/**
 * @brief table: sync current cell
 *
 * @param[in] table table
 * @param[in] row_out row out
 * @param[in] column_out column out
 * @return 0 on success, -1 on failure
 */

/**
 * @brief Add_ series from graph
 *
 * @param[in] backend_widget backend widget
 * @brief Set off source of switch backend
 *
 * @param[in] sw sw
 * @param[in] source Image source
 * @return 0 on success, -1 on failure
 */

int picoui_backend_switch_set_off_source(struct picoui_switch *sw, struct picoui_image_source *source);

/**
 * @brief Set on source of switch backend
 *
 * @param[in] sw sw
 * @param[in] source Image source
 * @return 0 on success, -1 on failure
 */

int picoui_backend_switch_set_on_source(struct picoui_switch *sw, struct picoui_image_source *source);

/**
 * @brief Set knob source of switch backend
 *
 * @param[in] sw sw
 * @param[in] source Image source
 * @return 0 on success, -1 on failure
 */

int picoui_backend_switch_set_knob_source(struct picoui_switch *sw, struct picoui_image_source *source);

/**
 * @brief Set horizontal of switch backend
 *
 * @param[in] sw sw
 * @param[in] horizontal horizontal
 * @return 0 on success, -1 on failure
 */

int picoui_backend_switch_set_horizontal(struct picoui_switch *sw, int horizontal);

/**
 * @brief Get horizontal from switch backend
 *
 * @param[out] sw sw
 * @param[in] horizontal horizontal
 * @return The property value, negative on error
 */

int picoui_backend_switch_get_horizontal(struct picoui_switch *sw, int *horizontal);

/**
 * @brief Set direction of switch backend
 *
 * @param[in] sw sw
 * @param[in] direction direction
 * @return 0 on success, -1 on failure
 */

int picoui_backend_switch_set_direction(struct picoui_switch *sw, int direction);

/**
 * @brief Get direction from switch backend
 *
 * @param[out] sw sw
 * @param[in] direction direction
 * @return The property value, negative on error
 */

int picoui_backend_switch_get_direction(struct picoui_switch *sw, int *direction);

/**
 * @brief Set disabled of switch backend
 *
 * @param[in] sw sw
 * @param[in] disabled disabled
 * @return 0 on success, -1 on failure
 */

int picoui_backend_switch_set_disabled(struct picoui_switch *sw, int disabled);

/**
 * @brief Get disabled from switch backend
 *
 * @param[out] sw sw
 * @param[in] disabled disabled
 * @return The property value, negative on error
 */

int picoui_backend_switch_get_disabled(struct picoui_switch *sw, int *disabled);

/**
 * @brief switch: can navigate
 *
 * @param[in] sw sw
 * @param[in] direction direction
 * @param[in] can_navigate can navigate
 * @return 0 on success, -1 on failure
 */

int picoui_backend_switch_can_navigate(struct picoui_switch *sw, int direction, int *can_navigate);

/**
 * @brief switch: navigate
 *
 * @param[in] sw sw
 * @param[in] direction direction
 * @return 0 on success, -1 on failure
 */

int picoui_backend_switch_navigate(struct picoui_switch *sw, int direction);

/**
 * @brief Set items of list backend
 *
 * @param[in] backend_widget backend widget
 * @param[in] item_ids item ids
 * @param[in] items items
 * @param[in] item_count item count
 * @return 0 on success, -1 on failure
 */

int tinyui_list_set_items(void *backend_widget,
                          const char *const *item_ids,
                          const unsigned char *const *items,
                          int item_count);

/**
 * @brief Set item height of list backend
 *
 * @param[in] backend_widget backend widget
 * @param[in] item_height item height
 * @return 0 on success, -1 on failure
 */

int tinyui_list_set_item_height(void *backend_widget, int item_height);

/**
 * @brief Set padding group of list backend
 *
 * @param[in] backend_widget backend widget
 * @param[in] top Top padding
 * @param[in] bottom Bottom padding
 * @param[in] left Left padding
 * @param[in] right Right padding
 * @return 0 on success, -1 on failure
 */

int tinyui_list_set_padding_group(void *backend_widget,
                                  int top,
                                  int bottom,
                                  int left,
                                  int right);

/**
 * @brief Set margin group of list backend
 *
 * @param[in] backend_widget backend widget
 * @param[in] top Top padding
 * @param[in] bottom Bottom padding
 * @param[in] left Left padding
 * @param[in] right Right padding
 * @return 0 on success, -1 on failure
 */

int tinyui_list_set_margin_group(void *backend_widget,
                                 int top,
                                 int bottom,
                                 int left,
                                 int right);

/**
 * @brief Set text color of list backend
 *
 * @param[in] backend_widget backend widget
 * @param[in] rgb RGB color value (0xRRGGBB)
 * @return 0 on success, -1 on failure
 */

int tinyui_list_set_text_color(void *backend_widget, unsigned int rgb);

/**
 * @brief Set bg color of list backend
 *
 * @param[in] backend_widget backend widget
 * @param[in] rgb RGB color value (0xRRGGBB)
 * @return 0 on success, -1 on failure
 */

int tinyui_list_set_bg_color(void *backend_widget, unsigned int rgb);

/**
 * @brief Set select color of list backend
 *
 * @param[in] backend_widget backend widget
 * @param[in] rgb RGB color value (0xRRGGBB)
 * @return 0 on success, -1 on failure
 */

int tinyui_list_set_select_color(void *backend_widget, unsigned int rgb);

/**
 * @brief Set align of list backend
 *
 * @param[in] backend_widget backend widget
 * @param[in] align align
 * @return 0 on success, -1 on failure
 */

int tinyui_list_set_align(void *backend_widget, enum picoui_align align);

/**
 * @brief Set item widget of list backend
 *
 * @param[in] backend_widget backend widget
 * @param[in] index Index
 * @param[in] item_widget_backend item widget backend
 * @return 0 on success, -1 on failure
 */

int tinyui_list_set_item_widget(void *backend_widget,
                                int index,
                                void *item_widget_backend);

/**
 * @brief Set style class of widget backend
 *
 * @param[in] backend_widget backend widget
 * @param[in] style_class style class
 * @return 0 on success, -1 on failure
 */

/**
 * @brief set: image source
 *
 * @param[in] backend_widget backend widget
 * @param[in] source Image source
 * @return 0 on success, -1 on failure
 */

int picoui_backend_set_image_source(void *backend_widget, struct picoui_image_source *source);

/**
 * @brief Set mask color of image backend
 *
 * @param[in] backend_widget backend widget
 * @param[in] rgb RGB color value (0xRRGGBB)
 * @return 0 on success, -1 on failure
 */

int picoui_backend_image_set_mask_color(void *backend_widget, unsigned int rgb);

/**
 * @brief Set flex grow of widget backend
 *
 * @param[in] widget Widget instance
 * @param[in] grow grow
 * @return 0 on success, -1 on failure
 */

/**
 * @brief emit: value changed
 *
 * @param[in] cb cb
 * @param[in] widget Widget instance
 * @param[in] value Value
 * @param[in] user_data User data pointer
 */

void tinyui_widget_emit_value_changed(picoui_value_changed_cb cb,
                                      struct picoui_widget *widget,
                                      int value,
                                      void *user_data);

/**
 * @brief emit: event
 *
 * @param[in] cb cb
 * @param[in] widget Widget instance
 * @param[in] user_data User data pointer
 */

void tinyui_widget_emit_event(picoui_event_cb cb,
                              struct picoui_widget *widget,
                              void *user_data);

/**
 * @brief widget: dispatch native signal
 *
 * @param[in] backend_widget backend widget
 * @param[in] native_signal native signal
 * @param[in] native_value native value
 * @return 0 on success, -1 on failure
 */


/**
 * @brief emit: clicked
 *
 * @param[in] cb cb
 * @param[in] widget Widget instance
 * @param[in] user_data User data pointer
 */

void tinyui_widget_emit_clicked(picoui_event_cb cb,
                                struct picoui_widget *widget,
                                void *user_data);

/**
 * @brief widget: claim focus
 *
 * @param[in] backend_widget backend widget
 * @return 0 on success, -1 on failure
 */

int tinyui_widget_claim_backend_focus(void *backend_widget);

/**
 * @brief widget: release focus
 *
 * @param[in] backend_widget backend widget
 * @return 0 on success, -1 on failure
 */

int tinyui_widget_release_backend_focus(void *backend_widget);

/**
 * @brief widget: init data model
 *
 * @param[in] backend backend
 */

void tinyui_widget_init_data_model(struct picoui_backend_widget *backend);

#endif
