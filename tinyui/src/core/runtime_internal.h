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
 *   - Layout cache structs (embedded in struct tinyui_backend_widget)
 *   - App lifecycle declarations
 *
 * NOTE: The shared enums reside here rather than in internal.h because
 * they are used as members of struct tinyui_backend_widget, which is
 * also defined here.  Putting them in internal.h would create a circular
 * dependency since internal.h includes this header.
 *
 * NOTE: struct tinyui_backend_layout_window_state and
 * struct tinyui_backend_layout_child_state also reside here because they
 * are embedded by-value in struct tinyui_backend_widget.
 */

#ifndef TINYUI_RUNTIME_INTERNAL_H
#define TINYUI_RUNTIME_INTERNAL_H

#include <stdint.h>

#include "widget.h"
#include "window.h"

struct tinyui_app;
struct tinyui_theme;
struct tinyui_widget;
struct tinyui_window;
struct tinyui_image_source;
struct ld_scene_t;

/* ── Constants ─────────────────────────────────────────────────── */

#define TINYUI_BACKEND_LAYOUT_MAX_TRACKS 16
#define TINYUI_BACKEND_LIST_MAX_ITEMS 16

/* ── Shared enums ──────────────────────────────────────────────── */

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

enum tinyui_backend_signal {
    TINYUI_BACKEND_SIGNAL_NONE = 0,
    TINYUI_BACKEND_SIGNAL_VALUE_CHANGED,
    TINYUI_BACKEND_SIGNAL_PRESSED,
    TINYUI_BACKEND_SIGNAL_RELEASED,
};

/* ── Layout cache (embedded in struct tinyui_backend_widget) ───── */

struct tinyui_backend_layout_window_state {
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
    int16_t grid_cols[TINYUI_BACKEND_LAYOUT_MAX_TRACKS];
    int16_t grid_rows[TINYUI_BACKEND_LAYOUT_MAX_TRACKS];
    int16_t grid_col_count;
    int16_t grid_row_count;
    int16_t grid_row_gap;
    int16_t grid_col_gap;
    int16_t grid_col_align;
    int16_t grid_row_align;
};

struct tinyui_backend_layout_child_state {
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

struct tinyui_backend_widget {
    struct tinyui_app *owner;
    struct tinyui_widget *host_widget;
    struct tinyui_backend_widget *root;
    struct tinyui_backend_widget *parent;
    struct tinyui_backend_widget *first_child;
    struct tinyui_backend_widget *next_sibling;
    const char *id;
    enum tinyui_backend_widget_kind kind;
    const char *text;
    const char *style_class;
    const void *font;
    void *user_data;
    struct ld_scene_t *ld_event_bridge_scene;
    void *ld_event_bridge_sender;
    struct tinyui_backend_widget *ld_event_bridge_next;
    struct tinyui_image_source *image_source;
    int value;
    struct tinyui_theme *theme;
    void *ld_widget;
    uint16_t ld_name_id;
    const char *list_item_ids[TINYUI_BACKEND_LIST_MAX_ITEMS];
    uint16_t list_item_count;
    int16_t edit_result_on_finish;
    struct tinyui_backend_layout_window_state window_layout;
    struct tinyui_backend_layout_child_state child_layout;
};

struct tinyui_backend_app_state {
    struct tinyui_theme *theme;
    struct ld_scene_t *ld_scene;
    uint16_t next_ld_name_id;
    void *runtime_state;
};

/* ── Test snapshot forward declarations ────────────────────────── */

struct tinyui_progress_wheel_test_dispose_snapshot;
struct tinyui_table_test_dispose_snapshot;

/* ── App backend lifecycle ─────────────────────────────────────── */

int tinyui_backend_app_init(struct tinyui_app *app);
int tinyui_backend_app_run(struct tinyui_app *app, struct tinyui_window *window);
void tinyui_backend_app_shutdown(struct tinyui_app *app);

/* ── Test-only helpers ─────────────────────────────────────────── */

void tinyui_backend_progress_wheel_test_reset_state(void);
void tinyui_backend_progress_wheel_test_fail_next_set_percent(void);
int tinyui_backend_progress_wheel_test_take_last_dispose_snapshot(
    struct tinyui_progress_wheel_test_dispose_snapshot *snapshot);
void tinyui_backend_progress_wheel_test_capture_dispose_snapshot(
    struct tinyui_backend_widget *backend,
    int detach_result,
    int unbind_result);
void tinyui_backend_table_test_fail_next_set_keyboard_binding(void);
void tinyui_backend_table_test_reset_state(void);
int tinyui_backend_table_test_take_last_dispose_snapshot(
    struct tinyui_table_test_dispose_snapshot *snapshot);

#endif /* TINYUI_RUNTIME_INTERNAL_H */