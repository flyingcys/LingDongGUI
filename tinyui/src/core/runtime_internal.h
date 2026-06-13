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

/**
 * @file runtime_internal.h
 * @brief Runtime/backend internal types shared across the framework.
 *
 * This header contains:
 *   - Shared enums (widget kind, signal, data truth, data source)
 *   - Backend widget tree and app state structs
 *   - Layout cache structs (embedded in struct picoui_backend_widget)
 *   - Runtime evidence flags
 *   - App lifecycle declarations
 *
 * NOTE: The shared enums reside here rather than in internal.h because
 * they are used as members of struct picoui_backend_widget, which is
 * also defined here.  Putting them in internal.h would create a circular
 * dependency since internal.h includes this header.
 *
 * NOTE: struct picoui_backend_layout_window_state and
 * struct picoui_backend_layout_child_state also reside here because they
 * are embedded by-value in struct picoui_backend_widget.
 */

#ifndef PICOUI_RUNTIME_INTERNAL_H
#define PICOUI_RUNTIME_INTERNAL_H

#include <stdint.h>

#include "widget.h"
#include "window.h"

struct picoui_app;
struct picoui_theme;
struct picoui_widget;
struct picoui_window;
struct picoui_image_source;
struct ld_scene_t;

/* ── Constants ─────────────────────────────────────────────────── */

#define PICOUI_BACKEND_LAYOUT_MAX_TRACKS 16
#define PICOUI_BACKEND_LIST_MAX_ITEMS 16

/* ── Shared enums ──────────────────────────────────────────────── */

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

/* ── Evidence flags ────────────────────────────────────────────── */

enum picoui_backend_runtime_evidence_flags {
    PICOUI_BACKEND_EVIDENCE_EXCLUDE_FORMAL_MAPPING = 1 << 0,
    PICOUI_BACKEND_EVIDENCE_ALLOW_SMOKE_LAYOUT = 1 << 1,
};

/* ── Layout cache (embedded in struct picoui_backend_widget) ───── */

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

/* ── Backend widget tree ───────────────────────────────────────── */

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

/* ── Test snapshot forward declarations ────────────────────────── */

struct picoui_progress_wheel_test_dispose_snapshot;
struct picoui_table_test_dispose_snapshot;

/* ── App backend lifecycle ─────────────────────────────────────── */

int picoui_backend_app_init(struct picoui_app *app);
int picoui_backend_app_run(struct picoui_app *app, struct picoui_window *window);
void picoui_backend_app_shutdown(struct picoui_app *app);

/* ── Test-only helpers ─────────────────────────────────────────── */

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

#endif /* PICOUI_RUNTIME_INTERNAL_H */