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

#include "picoui/image.h"
#include "picoui/native.h"
#include "picoui/widget.h"
#include "picoui/window.h"

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
    enum picoui_flex_flow flex_flow;
    enum picoui_align flex_main_align;
    enum picoui_align flex_cross_align;
    enum picoui_align flex_track_align;
    int padding;
    int padding_left;
    int padding_top;
    int padding_right;
    int padding_bottom;
    int has_explicit_flex_padding;
    int grid_padding_left;
    int grid_padding_top;
    int grid_padding_right;
    int grid_padding_bottom;
    int has_explicit_grid_padding;
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
    uint32_t last_native_signal;
    uint64_t last_native_value;
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
    int last_window_switch_mode;
    unsigned int last_window_switch_duration_ms;
};

struct picoui_progress_wheel_test_dispose_snapshot;

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
int picoui_backend_runtime_step(struct picoui_app *app);

/**
 * @brief Shutdown app backend
 *
 * @param[in] app Application instance
 */

void picoui_backend_app_shutdown(struct picoui_app *app);

/**
 * @brief widget: apply style
 *
 * @param[in] backend_widget backend widget
 * @param[in] part part
 * @param[in] state State value
 * @param[in] bg_color Background color
 * @param[in] text_color Text color
 * @param[in] border_color border color
 * @return 0 on success, -1 on failure
 */

int picoui_backend_widget_apply_style(void *backend_widget,
                                      enum picoui_part part,
                                      enum picoui_state state,
                                      unsigned int bg_color,
                                      unsigned int text_color,
                                      unsigned int border_color);

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
 * @brief Set day names of calendar backend
 *
 * @param[in] backend_widget backend widget
 * @param[in] day_names[7 day names[7
 * @return 0 on success, -1 on failure
 */

int picoui_backend_calendar_set_day_names(void *backend_widget, const char *const day_names[7]);

/**
 * @brief Set date of calendar backend
 *
 * @param[in] backend_widget backend widget
 * @param[in] year year
 * @param[in] month month
 * @param[in] day day
 * @return 0 on success, -1 on failure
 */

int picoui_backend_calendar_set_date(void *backend_widget, int year, int month, int day);

/**
 * @brief Get date from calendar backend
 *
 * @param[in] backend_widget backend widget
 * @param[in] year year
 * @param[in] month month
 * @param[in] day day
 * @return The property value, negative on error
 */

int picoui_backend_calendar_get_date(void *backend_widget, int *year, int *month, int *day);

/**
 * @brief Set header visible of calendar backend
 *
 * @param[in] backend_widget backend widget
 * @param[in] visible Visibility state
 * @return 0 on success, -1 on failure
 */

int picoui_backend_calendar_set_header_visible(void *backend_widget, int visible);

/**
 * @brief Get header visible from calendar backend
 *
 * @param[in] backend_widget backend widget
 * @return The property value, negative on error
 */

int picoui_backend_calendar_get_header_visible(void *backend_widget);

/**
 * @brief Set header format of calendar backend
 *
 * @param[in] backend_widget backend widget
 * @param[in] format Format string
 * @return 0 on success, -1 on failure
 */

int picoui_backend_calendar_set_header_format(void *backend_widget, const char *format);

/**
 * @brief Set bg color of calendar backend
 *
 * @param[in] backend_widget backend widget
 * @param[in] rgb RGB color value (0xRRGGBB)
 * @return 0 on success, -1 on failure
 */

int picoui_backend_calendar_set_bg_color(void *backend_widget, unsigned int rgb);

/**
 * @brief Set item color of calendar backend
 *
 * @param[in] backend_widget backend widget
 * @param[in] rgb RGB color value (0xRRGGBB)
 * @return 0 on success, -1 on failure
 */

int picoui_backend_calendar_set_item_color(void *backend_widget, unsigned int rgb);

/**
 * @brief Set text color of calendar backend
 *
 * @param[in] backend_widget backend widget
 * @param[in] rgb RGB color value (0xRRGGBB)
 * @return 0 on success, -1 on failure
 */

int picoui_backend_calendar_set_text_color(void *backend_widget, unsigned int rgb);

/**
 * @brief Set use system date of calendar backend
 *
 * @param[in] backend_widget backend widget
 * @param[in] enabled Enable state
 * @return 0 on success, -1 on failure
 */

int picoui_backend_calendar_set_use_system_date(void *backend_widget, int enabled);

/**
 * @brief Get use system date from calendar backend
 *
 * @param[in] backend_widget backend widget
 * @param[in] enabled Enable state
 * @return The property value, negative on error
 */

int picoui_backend_calendar_get_use_system_date(void *backend_widget, int *enabled);

/**
 * @brief Get header format from calendar backend
 *
 * @param[in] backend_widget backend widget
 */

const char *picoui_backend_calendar_get_header_format(void *backend_widget);

/**
 * @brief Get grid value from calendar backend
 *
 * @param[in] backend_widget backend widget
 * @param[in] week week
 * @param[in] weekday weekday
 * @return The property value, negative on error
 */

int picoui_backend_calendar_get_grid_value(void *backend_widget, int week, int weekday);

/**
 * @brief Check is current month cell of calendar
 *
 * @param[in] backend_widget backend widget
 * @param[in] week week
 * @param[in] weekday weekday
 * @return 0 on success, -1 on failure
 */

int picoui_backend_calendar_is_current_month_cell(void *backend_widget, int week, int weekday);

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

int picoui_backend_set_text(void *backend_widget, const char *text);

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
 * @brief keyboard: input ascii
 *
 * @param[in] backend_widget backend widget
 * @param[in] ascii ascii
 * @return 0 on success, -1 on failure
 */

int picoui_backend_keyboard_input_ascii(void *backend_widget, unsigned int ascii);

/**
 * @brief keyboard: navigate
 *
 * @param[in] backend_widget backend widget
 * @param[in] direction direction
 * @return 0 on success, -1 on failure
 */

int picoui_backend_keyboard_navigate(void *backend_widget, int direction);

/**
 * @brief keyboard: update
 *
 * @param[in] backend_widget backend widget
 * @return 0 on success, -1 on failure
 */

int picoui_backend_keyboard_update(void *backend_widget);

/**
 * @brief keyboard: button update
 *
 * @param[in] backend_widget backend widget
 * @param[in] key_code key code
 * @return 0 on success, -1 on failure
 */

int picoui_backend_keyboard_button_update(void *backend_widget, unsigned char key_code);

/**
 * @brief keyboard: click
 *
 * @param[in] backend_widget backend widget
 * @return 0 on success, -1 on failure
 */

int picoui_backend_keyboard_click(void *backend_widget);

/**
 * @brief keyboard: exit
 *
 * @param[in] backend_widget backend widget
 * @return 0 on success, -1 on failure
 */

int picoui_backend_keyboard_exit(void *backend_widget);

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
 * @brief Set keyboard binding of table backend
 *
 * @param[in] backend_widget backend widget
 * @param[in] keyboard_binding keyboard binding
 * @return 0 on success, -1 on failure
 */

int picoui_backend_table_set_keyboard_binding(void *backend_widget, unsigned int keyboard_binding);

/**
 * @brief Get keyboard binding from table backend
 *
 * @param[in] backend_widget backend widget
 * @param[in] keyboard_binding keyboard binding
 * @return The property value, negative on error
 */

int picoui_backend_table_get_keyboard_binding(void *backend_widget, unsigned int *keyboard_binding);

/**
 * @brief Set cell text of table backend
 *
 * @param[in] backend_widget backend widget
 * @param[in] row Row index
 * @param[in] column Column index
 * @param[in] text Text widget instance
 * @return 0 on success, -1 on failure
 */

int picoui_backend_table_set_cell_text(void *backend_widget, int row, int column, const char *text);

/**
 * @brief Get cell text from table backend
 *
 * @param[in] backend_widget backend widget
 * @param[in] row Row index
 * @param[in] column Column index
 */

const char *picoui_backend_table_get_cell_text(void *backend_widget, int row, int column);

/**
 * @brief Set cell editable of table backend
 *
 * @param[in] backend_widget backend widget
 * @param[in] row Row index
 * @param[in] column Column index
 * @param[in] editable editable
 * @param[in] text_max text max
 * @return 0 on success, -1 on failure
 */

int picoui_backend_table_set_cell_editable(void *backend_widget,
                                           int row,
                                           int column,
                                           int editable,
                                           unsigned int text_max);

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

int picoui_backend_table_set_item_image(void *backend_widget,
                                        int row,
                                        int column,
                                        int x,
                                        int y,
                                        struct picoui_image_source *source,
                                        unsigned int mask_color);

/**
 * @brief Set item button of table backend
 *
 * @param[in] backend_widget backend widget
 * @param[in] row Row index
 * @param[in] column Column index
 * @param[in] x X coordinate
 * @param[in] y Y coordinate
 * @param[in] release_source release source
 * @param[in] release_mask_color release mask color
 * @param[in] press_source press source
 * @param[in] press_mask_color press mask color
 * @param[in] checkable checkable
 * @return 0 on success, -1 on failure
 */

int picoui_backend_table_set_item_button(void *backend_widget,
                                         int row,
                                         int column,
                                         int x,
                                         int y,
                                         struct picoui_image_source *release_source,
                                         unsigned int release_mask_color,
                                         struct picoui_image_source *press_source,
                                         unsigned int press_mask_color,
                                         int checkable);

/**
 * @brief Set excel type of table backend
 *
 * @param[in] backend_widget backend widget
 * @return 0 on success, -1 on failure
 */

int picoui_backend_table_set_excel_type(void *backend_widget);

/**
 * @brief Set item width of table backend
 *
 * @param[in] backend_widget backend widget
 * @param[in] column Column index
 * @param[in] width Width in pixels
 * @return 0 on success, -1 on failure
 */

int picoui_backend_table_set_item_width(void *backend_widget, int column, int width);

/**
 * @brief Set item height of table backend
 *
 * @param[in] backend_widget backend widget
 * @param[in] row Row index
 * @param[in] height Height in pixels
 * @return 0 on success, -1 on failure
 */

int picoui_backend_table_set_item_height(void *backend_widget, int row, int height);

/**
 * @brief Set item color of table backend
 *
 * @param[in] backend_widget backend widget
 * @param[in] row Row index
 * @param[in] column Column index
 * @param[in] text_color Text color
 * @param[in] bg_color Background color
 * @return 0 on success, -1 on failure
 */

int picoui_backend_table_set_item_color(void *backend_widget,
                                        int row,
                                        int column,
                                        unsigned int text_color,
                                        unsigned int bg_color);

/**
 * @brief Set bg color of table backend
 *
 * @param[in] backend_widget backend widget
 * @param[in] bg_color Background color
 * @return 0 on success, -1 on failure
 */

int picoui_backend_table_set_bg_color(void *backend_widget, unsigned int bg_color);

/**
 * @brief Set item static text of table backend
 *
 * @param[in] backend_widget backend widget
 * @param[in] row Row index
 * @param[in] column Column index
 * @param[in] text Text widget instance
 * @return 0 on success, -1 on failure
 */

int picoui_backend_table_set_item_static_text(void *backend_widget, int row, int column, const char *text);

/**
 * @brief Set item font of table backend
 *
 * @param[in] backend_widget backend widget
 * @param[in] row Row index
 * @param[in] column Column index
 * @return 0 on success, -1 on failure
 */

int picoui_backend_table_set_item_font(void *backend_widget, int row, int column);

/**
 * @brief Set item align of table backend
 *
 * @param[in] backend_widget backend widget
 * @param[in] row Row index
 * @param[in] column Column index
 * @param[in] align align
 * @return 0 on success, -1 on failure
 */

int picoui_backend_table_set_item_align(void *backend_widget,
                                        int row,
                                        int column,
                                        enum picoui_align align);

/**
 * @brief Get item align from table backend
 *
 * @param[in] backend_widget backend widget
 * @param[in] row Row index
 * @param[in] column Column index
 * @return The property value, negative on error
 */

int picoui_backend_table_get_item_align(void *backend_widget, int row, int column);

/**
 * @brief Get item editable from table backend
 *
 * @param[in] backend_widget backend widget
 * @param[in] row Row index
 * @param[in] column Column index
 * @return The property value, negative on error
 */

int picoui_backend_table_get_item_editable(void *backend_widget, int row, int column);

/**
 * @brief table: navigate
 *
 * @param[in] backend_widget backend widget
 * @param[in] dir dir
 * @return 0 on success, -1 on failure
 */

int picoui_backend_table_navigate(void *backend_widget, enum picoui_native_nav_dir dir);

/**
 * @brief Get item region from table backend
 *
 * @param[in] backend_widget backend widget
 * @param[in] row Row index
 * @param[in] column Column index
 * @param[in] region_out region out
 * @return The property value, negative on error
 */

int picoui_backend_table_get_item_region(void *backend_widget, int row, int column, void *region_out);

/**
 * @brief Set selected cell of table backend
 *
 * @param[in] backend_widget backend widget
 * @param[in] row Row index
 * @param[in] column Column index
 * @return 0 on success, -1 on failure
 */

int picoui_backend_table_set_selected_cell(void *backend_widget, int row, int column);

/**
 * @brief Set current cell of table backend
 *
 * @param[in] backend_widget backend widget
 * @param[in] row Row index
 * @param[in] column Column index
 * @return 0 on success, -1 on failure
 */

int picoui_backend_table_set_current_cell(void *backend_widget, int row, int column);

/**
 * @brief table: sync current cell
 *
 * @param[in] table table
 * @param[in] row_out row out
 * @param[in] column_out column out
 * @return 0 on success, -1 on failure
 */

int picoui_backend_table_sync_current_cell(struct picoui_table *table, int *row_out, int *column_out);

/**
 * @brief table: bind host
 *
 * @param[in] backend_widget backend widget
 * @return 0 on success, -1 on failure
 */

int picoui_backend_table_bind_host(void *backend_widget);

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

int picoui_backend_list_set_items(void *backend_widget,
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

int picoui_backend_list_set_item_height(void *backend_widget, int item_height);

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

int picoui_backend_list_set_padding_group(void *backend_widget,
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

int picoui_backend_list_set_margin_group(void *backend_widget,
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

int picoui_backend_list_set_text_color(void *backend_widget, unsigned int rgb);

/**
 * @brief Set bg color of list backend
 *
 * @param[in] backend_widget backend widget
 * @param[in] rgb RGB color value (0xRRGGBB)
 * @return 0 on success, -1 on failure
 */

int picoui_backend_list_set_bg_color(void *backend_widget, unsigned int rgb);

/**
 * @brief Set select color of list backend
 *
 * @param[in] backend_widget backend widget
 * @param[in] rgb RGB color value (0xRRGGBB)
 * @return 0 on success, -1 on failure
 */

int picoui_backend_list_set_select_color(void *backend_widget, unsigned int rgb);

/**
 * @brief Set align of list backend
 *
 * @param[in] backend_widget backend widget
 * @param[in] align align
 * @return 0 on success, -1 on failure
 */

int picoui_backend_list_set_align(void *backend_widget, enum picoui_align align);

/**
 * @brief Set item widget of list backend
 *
 * @param[in] backend_widget backend widget
 * @param[in] index Index
 * @param[in] item_widget_backend item widget backend
 * @return 0 on success, -1 on failure
 */

int picoui_backend_list_set_item_widget(void *backend_widget,
                                        int index,
                                        void *item_widget_backend);

/**
 * @brief Set selected index of list backend
 *
 * @param[in] backend_widget backend widget
 * @param[in] index Index
 * @return 0 on success, -1 on failure
 */

int picoui_backend_list_set_selected_index(void *backend_widget, int index);

/**
 * @brief Get selected index from list backend
 *
 * @param[in] backend_widget backend widget
 * @return The property value, negative on error
 */

int picoui_backend_list_get_selected_index(void *backend_widget);

/**
 * @brief list: sync selected index
 *
 * @param[in] list List widget instance
 * @param[in] selected_index_out selected index out
 * @return 0 on success, -1 on failure
 */

int picoui_backend_list_sync_selected_index(struct picoui_list *list, int *selected_index_out);

/**
 * @brief Set style class of widget backend
 *
 * @param[in] backend_widget backend widget
 * @param[in] style_class style class
 * @return 0 on success, -1 on failure
 */

/**
 * @brief Set padding of widget backend
 *
 * @param[in] backend_widget backend widget
 * @param[in] padding padding
 * @return 0 on success, -1 on failure
 */

int picoui_backend_widget_set_padding(void *backend_widget, int padding);

/**
 * @brief widget: bind ld event bridge
 *
 * @param[in] backend_widget backend widget
 * @param[in] scene scene
 * @param[in] sender sender
 * @return 0 on success, -1 on failure
 */

int picoui_backend_widget_bind_ld_event_bridge(void *backend_widget,
                                               struct ld_scene_t *scene,
                                               void *sender);

/**
 * @brief widget: bind host
 *
 * @param[in] backend_widget backend widget
 * @param[in] widget Widget instance
 * @return 0 on success, -1 on failure
 */

int picoui_backend_widget_bind_host(void *backend_widget,
                                    struct picoui_widget *widget);

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
 * @brief Set flex flow of window backend
 *
 * @param[in] window Window instance
 * @param[in] flow flow
 * @return 0 on success, -1 on failure
 */

int picoui_backend_window_set_flex_flow(struct picoui_window *window, enum picoui_flex_flow flow);

/**
 * @brief Set flex align of window backend
 *
 * @param[in] window Window instance
 * @param[in] main_align main align
 * @param[in] cross_align cross align
 * @param[in] track_align track align
 * @return 0 on success, -1 on failure
 */

int picoui_backend_window_set_flex_align(struct picoui_window *window,
                                         enum picoui_align main_align,
                                         enum picoui_align cross_align,
                                         enum picoui_align track_align);

/**
 * @brief Set flex gap of window backend
 *
 * @param[in] window Window instance
 * @param[in] item_gap item gap
 * @param[in] track_gap track gap
 * @return 0 on success, -1 on failure
 */

int picoui_backend_window_set_flex_gap(struct picoui_window *window, int item_gap, int track_gap);

/**
 * @brief Set grid columns of window backend
 *
 * @param[in] window Window instance
 * @param[in] tracks tracks
 * @param[in] count Count
 * @return 0 on success, -1 on failure
 */

int picoui_backend_window_set_grid_columns(struct picoui_window *window, const int *tracks, int count);

/**
 * @brief Set grid rows of window backend
 *
 * @param[in] window Window instance
 * @param[in] tracks tracks
 * @param[in] count Count
 * @return 0 on success, -1 on failure
 */

int picoui_backend_window_set_grid_rows(struct picoui_window *window, const int *tracks, int count);

/**
 * @brief Set grid gap of window backend
 *
 * @param[in] window Window instance
 * @param[in] row_gap row gap
 * @param[in] col_gap col gap
 * @return 0 on success, -1 on failure
 */

int picoui_backend_window_set_grid_gap(struct picoui_window *window, int row_gap, int col_gap);

/**
 * @brief Set grid align of window backend
 *
 * @param[in] window Window instance
 * @param[in] col_align col align
 * @param[in] row_align row align
 * @return 0 on success, -1 on failure
 */

int picoui_backend_window_set_grid_align(struct picoui_window *window,
                                         enum picoui_align col_align,
                                         enum picoui_align row_align);

/**
 * @brief Set layout type of window backend
 *
 * @param[in] window Window instance
 * @param[in] type Type
 * @return 0 on success, -1 on failure
 */

int picoui_backend_window_set_layout_type(struct picoui_window *window,
                                          enum picoui_window_layout_type type);

/**
 * @brief Set padding of window backend
 *
 * @param[in] window Window instance
 * @param[in] left Left padding
 * @param[in] top Top padding
 * @param[in] right Right padding
 * @param[in] bottom Bottom padding
 * @return 0 on success, -1 on failure
 */

int picoui_backend_window_set_padding(struct picoui_window *window,
                                      int left,
                                      int top,
                                      int right,
                                      int bottom);

/**
 * @brief Set grid padding of window backend
 *
 * @param[in] window Window instance
 * @param[in] left Left padding
 * @param[in] top Top padding
 * @param[in] right Right padding
 * @param[in] bottom Bottom padding
 * @return 0 on success, -1 on failure
 */

int picoui_backend_window_set_grid_padding(struct picoui_window *window,
                                           int left,
                                           int top,
                                           int right,
                                           int bottom);

/**
 * @brief Set gap of window backend
 *
 * @param[in] window Window instance
 * @param[in] gap Gap in pixels
 * @return 0 on success, -1 on failure
 */

int picoui_backend_window_set_gap(struct picoui_window *window, int gap);

/**
 * @brief Set flex grow of widget backend
 *
 * @param[in] widget Widget instance
 * @param[in] grow grow
 * @return 0 on success, -1 on failure
 */

int picoui_backend_widget_set_flex_grow(struct picoui_widget *widget, int grow);

/**
 * @brief Set flex new track of widget backend
 *
 * @param[in] widget Widget instance
 * @param[in] new_track new track
 * @return 0 on success, -1 on failure
 */

int picoui_backend_widget_set_flex_new_track(struct picoui_widget *widget, int new_track);

/**
 * @brief Set ignore layout of widget backend
 *
 * @param[in] widget Widget instance
 * @param[in] ignore_layout ignore layout
 * @return 0 on success, -1 on failure
 */

int picoui_backend_widget_set_ignore_layout(struct picoui_widget *widget, int ignore_layout);

/**
 * @brief Set grid cell of widget backend
 *
 * @param[in] widget Widget instance
 * @param[in] col col
 * @param[in] row Row index
 * @param[in] col_span Column span count
 * @param[in] row_span Row span count
 * @param[in] x_align x align
 * @param[in] y_align y align
 * @return 0 on success, -1 on failure
 */

int picoui_backend_widget_set_grid_cell(struct picoui_widget *widget,
                                        int col,
                                        int row,
                                        int col_span,
                                        int row_span,
                                        enum picoui_align x_align,
                                        enum picoui_align y_align);

/**
 * @brief emit: value changed
 *
 * @param[in] cb cb
 * @param[in] widget Widget instance
 * @param[in] value Value
 * @param[in] user_data User data pointer
 */

void picoui_backend_emit_value_changed(picoui_value_changed_cb cb,
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

void picoui_backend_emit_event(picoui_event_cb cb,
                               struct picoui_widget *widget,
                               void *user_data);

/**
 * @brief widget: dispatch signal
 *
 * @param[in] backend_widget backend widget
 * @param[in] signal signal
 * @param[in] value Value
 * @param[in] cb cb
 * @param[in] widget Widget instance
 * @param[in] user_data User data pointer
 * @return 0 on success, -1 on failure
 */

int picoui_backend_widget_dispatch_signal(void *backend_widget,
                                          enum picoui_backend_signal signal,
                                          int value,
                                          picoui_value_changed_cb cb,
                                          struct picoui_widget *widget,
                                          void *user_data);

/**
 * @brief widget: dispatch event
 *
 * @param[in] backend_widget backend widget
 * @param[in] signal signal
 * @param[in] cb cb
 * @param[in] widget Widget instance
 * @param[in] user_data User data pointer
 * @return 0 on success, -1 on failure
 */

int picoui_backend_widget_dispatch_event(void *backend_widget,
                                         enum picoui_backend_signal signal,
                                         picoui_event_cb cb,
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

int picoui_backend_widget_dispatch_native_signal(void *backend_widget,
                                                 uint32_t native_signal,
                                                 uint64_t native_value);

/**
 * @brief widget: update value
 *
 * @param[in] backend_widget backend widget
 * @param[in] value Value
 * @param[in] cb cb
 * @param[in] widget Widget instance
 * @param[in] user_data User data pointer
 * @return 0 on success, -1 on failure
 */

int picoui_backend_widget_update_value(void *backend_widget,
                                       int value,
                                       picoui_value_changed_cb cb,
                                       struct picoui_widget *widget,
                                       void *user_data);

/**
 * @brief emit: clicked
 *
 * @param[in] cb cb
 * @param[in] widget Widget instance
 * @param[in] user_data User data pointer
 */

void picoui_backend_emit_clicked(picoui_event_cb cb,
                                 struct picoui_widget *widget,
                                 void *user_data);

/**
 * @brief widget: claim focus
 *
 * @param[in] backend_widget backend widget
 * @return 0 on success, -1 on failure
 */

int picoui_backend_widget_claim_focus(void *backend_widget);

/**
 * @brief widget: release focus
 *
 * @param[in] backend_widget backend widget
 * @return 0 on success, -1 on failure
 */

int picoui_backend_widget_release_focus(void *backend_widget);

/**
 * @brief widget: init data model
 *
 * @param[in] backend backend
 */

void picoui_backend_widget_init_data_model(struct picoui_backend_widget *backend);

#endif
