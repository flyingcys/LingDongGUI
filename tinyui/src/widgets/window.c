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

#include "internal.h"
#include "widget.h"
#include "window.h"
#include "../core/runtime_bridge.h"
#include "../../../src/gui/ldWindow.h"
#include "../../../src/porting/ldConfig.h"

#include <stdlib.h>
#include <string.h>

struct tinyui_image_source;

static int s_window_set_bg_color_fail_flag = 0;

#define TINYUI_WINDOW_LAYOUT_MAX_TRACKS TINYUI_BACKEND_LAYOUT_MAX_TRACKS

/* ── window-private RGB565 → RGB888 round-trip ───────────────────────────
 * The core helper tinyui_ld_color_to_rgb uses the bit-replication formula
 * ((c<<3)|(c>>2)) which gives slightly different values than the legacy
 * "* 255 / 31" formula that window's public get_color contract has used
 * since v2.0 and that test_tinyui_window's bg_color round-trip pins down
 * (expects 0x204462 from a 0x224466 round-trip).  Keep the legacy
 * formula window-private so the public contract is preserved while every
 * other widget shares the core helper. */
static unsigned int s_ld_color_to_rgb_legacy(unsigned int color)
{
    uint32_t red = (color >> 11) & 0x1FU;
    uint32_t green = (color >> 5) & 0x3FU;
    uint32_t blue = color & 0x1FU;

    red = (red * 255U) / 31U;
    green = (green * 255U) / 63U;
    blue = (blue * 255U) / 31U;
    return (red << 16) | (green << 8) | blue;
}

/* ── window-private flex/grid align mappings ─────────────────────────────
 * These mappings are window-private (flex_main / flex_cross / flex_track
 * / grid) and intentionally NOT merged into the core
 * tinyui_align_to_arm2d helper.  Keep them here. */
static ldFlexFlow_t s_flex_flow_to_ld(enum tinyui_flex_flow flow)
{
    switch (flow) {
    case TINYUI_FLEX_FLOW_COLUMN:
        return ldFlexFlowColumn;
    case TINYUI_FLEX_FLOW_ROW_WRAP:
        return ldFlexFlowRowWrap;
    case TINYUI_FLEX_FLOW_COLUMN_WRAP:
        return ldFlexFlowColumnWrap;
    case TINYUI_FLEX_FLOW_ROW_REVERSE:
        return ldFlexFlowRowReverse;
    case TINYUI_FLEX_FLOW_COLUMN_REVERSE:
        return ldFlexFlowColumnReverse;
    case TINYUI_FLEX_FLOW_ROW_WRAP_REVERSE:
        return ldFlexFlowRowWrapReverse;
    case TINYUI_FLEX_FLOW_COLUMN_WRAP_REVERSE:
        return ldFlexFlowColumnWrapReverse;
    case TINYUI_FLEX_FLOW_ROW:
    default:
        return ldFlexFlowRow;
    }
}

static ldFlexMainAlign_t s_flex_main_align_to_ld(enum tinyui_align align)
{
    switch (align) {
    case TINYUI_ALIGN_CENTER:
        return ldFlexMainAlignCenter;
    case TINYUI_ALIGN_END:
        return ldFlexMainAlignEnd;
    case TINYUI_ALIGN_SPACE_EVENLY:
        return ldFlexMainAlignSpaceEvenly;
    case TINYUI_ALIGN_SPACE_AROUND:
        return ldFlexMainAlignSpaceAround;
    case TINYUI_ALIGN_SPACE_BETWEEN:
        return ldFlexMainAlignSpaceBetween;
    case TINYUI_ALIGN_STRETCH:
    case TINYUI_ALIGN_START:
    default:
        return ldFlexMainAlignStart;
    }
}

static ldFlexCrossAlign_t s_flex_cross_align_to_ld(enum tinyui_align align)
{
    switch (align) {
    case TINYUI_ALIGN_CENTER:
        return ldFlexCrossAlignCenter;
    case TINYUI_ALIGN_END:
        return ldFlexCrossAlignEnd;
    case TINYUI_ALIGN_STRETCH:
    case TINYUI_ALIGN_SPACE_EVENLY:
    case TINYUI_ALIGN_SPACE_AROUND:
    case TINYUI_ALIGN_SPACE_BETWEEN:
    case TINYUI_ALIGN_START:
    default:
        return ldFlexCrossAlignStart;
    }
}

static ldFlexTrackAlign_t s_flex_track_align_to_ld(enum tinyui_align align)
{
    switch (align) {
    case TINYUI_ALIGN_CENTER:
        return ldFlexTrackAlignCenter;
    case TINYUI_ALIGN_END:
        return ldFlexTrackAlignEnd;
    case TINYUI_ALIGN_SPACE_BETWEEN:
        return ldFlexTrackAlignSpaceBetween;
    case TINYUI_ALIGN_SPACE_AROUND:
        return ldFlexTrackAlignSpaceAround;
    case TINYUI_ALIGN_SPACE_EVENLY:
        return ldFlexTrackAlignSpaceEvenly;
    case TINYUI_ALIGN_STRETCH:
    case TINYUI_ALIGN_START:
    default:
        return ldFlexTrackAlignStart;
    }
}

static ldGridAlign_t s_grid_align_to_ld(enum tinyui_align align)
{
    return (ldGridAlign_t)tinyui_native_align_to_ld_grid((enum tinyui_native_align)align);
}

static int16_t s_grid_track_to_ld(int value)
{
    if (value == 0) {
        return LD_GRID_TEMPLATE_LAST;
    }
    if (value == -2) {
        return LD_GRID_CONTENT;
    }
    if (value < 0) {
        return LD_GRID_FR((-value) - 1);
    }
    return (int16_t)value;
}

static int s_copy_grid_tracks(int *dst, int16_t *backend_dst, const int *src, int count)
{
    int i;

    if (dst == 0 || backend_dst == 0 || src == 0 || count <= 0 || count > TINYUI_LAYOUT_MAX_TRACKS) {
        return -1;
    }

    for (i = 0; i < count; ++i) {
        dst[i] = src[i];
        backend_dst[i] = s_grid_track_to_ld(src[i]);
    }
    for (; i < TINYUI_LAYOUT_MAX_TRACKS; ++i) {
        dst[i] = 0;
        backend_dst[i] = LD_GRID_TEMPLATE_LAST;
    }
    return 0;
}

/* C3-T4: the legacy `struct tinyui_window_backend_host` wrapper has been
 * deleted — every window (window.c and background.c paths alike) now folds
 * its binding state directly onto struct tinyui_window.  The accessors below
 * read the folded fields exclusively. */

static void tinyui_window_sync_padding(struct tinyui_window *window);

static int tinyui_window_do_set_layout_type(struct tinyui_window *window,
                                                enum tinyui_window_layout_type type);

static int tinyui_window_do_set_flex_contract(struct tinyui_window *window,
                                                  enum tinyui_flex_flow flow,
                                                  enum tinyui_align main_align,
                                                  enum tinyui_align cross_align,
                                                  enum tinyui_align track_align,
                                                  int item_gap,
                                                  int track_gap);

static int tinyui_window_do_set_gap(struct tinyui_window *window, int gap);

static ldWindow_t *tinyui_window_ld_of(struct tinyui_window *window)
{
    ldBase_t *ld_base;

    /* C2: read the folded widget fields directly to resolve the
     * underlying ld widget. */
    if (window == 0 || window->widget.ld_widget == 0) {
        return 0;
    }

    if (window->widget.kind != TINYUI_BACKEND_WIDGET_WINDOW &&
        window->widget.kind != TINYUI_BACKEND_WIDGET_BACKGROUND) {
        return 0;
    }

    ld_base = (ldBase_t *)window->widget.ld_widget;
    if (ld_base->widgetType != widgetTypeWindow &&
        ld_base->widgetType != widgetTypeBackground) {
        return 0;
    }

    return (ldWindow_t *)ld_base;
}

static ldLayoutType_t tinyui_window_layout_type_to_ld(enum tinyui_window_layout_type type)
{
    switch (type) {
    case TINYUI_WINDOW_LAYOUT_FLEX:
        return layoutFlex;
    case TINYUI_WINDOW_LAYOUT_GRID:
        return layoutGrid;
    case TINYUI_WINDOW_LAYOUT_NONE:
    default:
        return layoutNone;
    }
}

int tinyui_window_apply_uniform_padding(struct tinyui_window *window, int padding)
{
    ldWindow_t *ld_window = tinyui_window_ld_of(window);

    if (window == 0 || ld_window == 0 || padding < 0) {
        return -1;
    }

    /* Folded source-of-truth on the window struct */
    window->padding_left = (int16_t)padding;
    window->padding_top = (int16_t)padding;
    window->padding_right = (int16_t)padding;
    window->padding_bottom = (int16_t)padding;
    window->has_explicit_flex_padding = 0;
    window->grid_padding_left = (int16_t)padding;
    window->grid_padding_top = (int16_t)padding;
    window->grid_padding_right = (int16_t)padding;
    window->grid_padding_bottom = (int16_t)padding;
    window->has_explicit_grid_padding = 0;

    tinyui_window_sync_padding(window);
    return 0;
}

int tinyui_window_apply_explicit_grid_padding(struct tinyui_window *window,
                                              int left,
                                              int top,
                                              int right,
                                              int bottom)
{
    ldWindow_t *ld_window = tinyui_window_ld_of(window);
    ldLayoutType_t layout_type;

    if (window == 0 || ld_window == 0
        || left < 0 || top < 0 || right < 0 || bottom < 0) {
        return -1;
    }

    /* C2: write the folded fields on the window struct as the
     * authoritative source. */
    window->grid_padding_left = (int16_t)left;
    window->grid_padding_top = (int16_t)top;
    window->grid_padding_right = (int16_t)right;
    window->grid_padding_bottom = (int16_t)bottom;
    window->has_explicit_grid_padding = 1;

    layout_type = ld_window->layoutTpye;
    ldWindowSetGridPadding(ld_window, (ldPadding_t){
        .left = (int16_t)left,
        .top = (int16_t)top,
        .right = (int16_t)right,
        .bottom = (int16_t)bottom,
    });
    ld_window->layoutTpye = layout_type;
    return 0;
}

static void tinyui_window_set_defaults(struct tinyui_window *window, const char *id)
{
    window->id = id;
    window->widget.visible = 1;
    window->widget.enabled = 1;
    window->flex_flow = TINYUI_FLEX_FLOW_ROW;
    window->flex_main_align = TINYUI_ALIGN_START;
    window->flex_cross_align = TINYUI_ALIGN_START;
    window->flex_track_align = TINYUI_ALIGN_START;
    window->grid_col_align = TINYUI_ALIGN_START;
    window->grid_row_align = TINYUI_ALIGN_START;
}

static void tinyui_window_do_free_internal(struct tinyui_window *window)
{
    struct tinyui_app *app_state;
    ldWindow_t *ld_win;
    int clears_scene_root = 0;

    if (window == 0) {
        free(window);
        return;
    }

    app_state = window->widget.owner != 0
        ? tinyui_runtime_bridge_backend_state(window->widget.owner)
        : 0;

    /* Save ld_win before detach clears widget->ld_widget. */
    ld_win = (ldWindow_t *)window->widget.ld_widget;

    /* C2: clear runtime bridge state via the folded widget directly.
     * No wrapper to unbind. */
    window->widget.ld_event_bridge_scene = 0;
    window->widget.ld_event_bridge_sender = 0;
    window->widget.ld_event_bridge_next = 0;
    if (ld_win != 0) {
        ((ldBase_t *)ld_win)->pInfo = 0;
    }

    /* C2: detach the ld node from its parent directly. */
    if (ld_win != 0 && ldBaseGetParent((ldBase_t *)ld_win) != 0) {
        ldBaseNodeRemove((ldBase_t *)ld_win);
    }
    window->widget.ld_widget = 0;

    if (ld_win != 0
        && app_state != 0
        && app_state->ld_scene != 0
        && app_state->ld_scene->ptNodeRoot == (arm_2d_control_node_t *)ld_win) {
        clears_scene_root = 1;
    }
    if (app_state != 0 && app_state->ld_scene != 0 && ld_win != 0) {
        ldWindow_depose(app_state->ld_scene, ld_win);
        if (clears_scene_root) {
            app_state->ld_scene->ptNodeRoot = 0;
        } else {
            /* C1 phantom root: if no user windows remain, depose phantom too. */
            ldBase_t *phantom = (ldBase_t *)app_state->ld_scene->ptNodeRoot;
            if (phantom != NULL && ldBaseGetChildList(phantom) == NULL) {
                ldWindow_depose(app_state->ld_scene, (ldWindow_t *)phantom);
                app_state->ld_scene->ptNodeRoot = 0;
            }
        }
    }

    /* C3-T4: no wrapper to free — single free of the window struct. */
    free(window->padding_group_storage);
    free(window);
}

void tinyui_window_test_fail_next_set_bg_color(void)
{
    s_window_set_bg_color_fail_flag = 1;
}

static int tinyui_window_apply_padding_group(struct tinyui_window *window,
                                              int left,
                                              int top,
                                              int right,
                                              int bottom)
{
    if (window == 0) {
        return -1;
    }

    if (tinyui_window_ld_of(window) == 0) {
        return -1;
    }

    /* C2: padding-group state collapses onto the folded fields.
     * has_explicit_flex_padding (which already exists on struct
     * tinyui_window per C1-T1) now subsumes has_padding_group. */
    window->padding_left = (int16_t)left;
    window->padding_top = (int16_t)top;
    window->padding_right = (int16_t)right;
    window->padding_bottom = (int16_t)bottom;
    window->has_explicit_flex_padding = 1;

    tinyui_window_sync_padding(window);
    return 0;
}

static void tinyui_window_sync_padding(struct tinyui_window *window)
{
    ldLayoutType_t layout_type;
    ldPadding_t flex_padding;
    ldPadding_t grid_padding;
    ldWindow_t *ld_window;
    ldPadding_t padding_group;
    int has_padding_group;

    if (window == 0) {
        return;
    }

    ld_window = tinyui_window_ld_of(window);
    if (ld_window == 0) {
        return;
    }

    /* C3-T4: source padding from the folded fields (single source of truth). */
    flex_padding = (ldPadding_t){
        .left = window->padding_left,
        .top = window->padding_top,
        .right = window->padding_right,
        .bottom = window->padding_bottom,
    };
    grid_padding = (ldPadding_t){
        .left = window->grid_padding_left,
        .top = window->grid_padding_top,
        .right = window->grid_padding_right,
        .bottom = window->grid_padding_bottom,
    };

    /* padding-group: derive from folded fields. ldWindowSetPaddingGroup stores
     * the pointer, so the storage must outlive this call — use the heap-
     * allocated padding_group_storage that persists for the window's life. */
    has_padding_group = window->has_explicit_flex_padding;
    if (window->padding_group_storage == 0) {
        window->padding_group_storage = calloc(1, sizeof(ldPadding_t));
    }
    if (window->padding_group_storage != 0) {
        ldPadding_t *stored = (ldPadding_t *)window->padding_group_storage;
        stored->left   = window->padding_left;
        stored->top    = window->padding_top;
        stored->right  = window->padding_right;
        stored->bottom = window->padding_bottom;
        padding_group  = *stored;
    }

    layout_type = ld_window->layoutTpye;
    if (has_padding_group) {
        flex_padding = padding_group;
        grid_padding = padding_group;
    }
    ldWindowSetPadding(ld_window, flex_padding);
    ldWindowSetGridPadding(ld_window, grid_padding);
    if (has_padding_group) {
        /* ldWindow stores the pointer; point at persistent storage. */
        ldWindowSetPaddingGroup(ld_window, (ldPadding_t *)window->padding_group_storage);
    }
    ld_window->layoutTpye = layout_type;
}

static int tinyui_window_do_set_layout_type(struct tinyui_window *window,
                                                enum tinyui_window_layout_type type)
{
    ldWindow_t *ld_window = tinyui_window_ld_of(window);

    if (window == 0 || ld_window == 0
        || (type != TINYUI_WINDOW_LAYOUT_NONE
            && type != TINYUI_WINDOW_LAYOUT_FLEX
            && type != TINYUI_WINDOW_LAYOUT_GRID)) {
        return -1;
    }

    ldWindowSetLayout(ld_window, tinyui_window_layout_type_to_ld(type));
    tinyui_window_sync_padding(window);
    return 0;
}

static int tinyui_window_do_set_flex_contract(struct tinyui_window *window,
                                                  enum tinyui_flex_flow flow,
                                                  enum tinyui_align main_align,
                                                  enum tinyui_align cross_align,
                                                  enum tinyui_align track_align,
                                                  int item_gap,
                                                  int track_gap)
{
    ldWindow_t *ld_window = tinyui_window_ld_of(window);

    if (window == 0 || ld_window == 0 || item_gap < 0 || track_gap < 0) {
        return -1;
    }

    window->flex_flow = flow;
    window->flex_main_align = main_align;
    window->flex_cross_align = cross_align;
    window->flex_track_align = track_align;
    window->flex_item_gap = item_gap;
    window->flex_track_gap = track_gap;

    ldWindowSetFlexFlow(ld_window, s_flex_flow_to_ld(flow));
    ldWindowSetFlexAlign(ld_window,
                         s_flex_main_align_to_ld(main_align),
                         s_flex_cross_align_to_ld(cross_align));
    ldWindowSetFlexTrackAlign(ld_window, s_flex_track_align_to_ld(track_align));
    ldWindowSetFlexGap(ld_window, (int16_t)item_gap, (int16_t)track_gap);
    tinyui_window_sync_padding(window);
    return 0;
}

static int tinyui_window_do_set_gap(struct tinyui_window *window, int gap)
{
    ldWindow_t *ld_window = tinyui_window_ld_of(window);
    ldLayoutType_t layout_type;

    if (window == 0 || ld_window == 0 || gap < 0) {
        return -1;
    }

    window->flex_item_gap = gap;
    window->flex_track_gap = gap;

    layout_type = ld_window->layoutTpye;
    ldWindowSetGap(ld_window, (int16_t)gap);
    ld_window->layoutTpye = layout_type;
    return 0;
}

int tinyui_window_apply_flex_flow(struct tinyui_window *window, enum tinyui_flex_flow flow)
{
    ldWindow_t *ld_window = tinyui_window_ld_of(window);

    if (window == 0 || ld_window == 0) {
        return -1;
    }

    window->flex_flow = flow;
    ldWindowSetFlexFlow(ld_window, s_flex_flow_to_ld(flow));
    tinyui_window_sync_padding(window);
    return 0;
}

int tinyui_window_apply_flex_align(struct tinyui_window *window,
                                   enum tinyui_align main_align,
                                   enum tinyui_align cross_align,
                                   enum tinyui_align track_align)
{
    ldWindow_t *ld_window = tinyui_window_ld_of(window);

    if (window == 0 || ld_window == 0) {
        return -1;
    }

    window->flex_main_align = main_align;
    window->flex_cross_align = cross_align;
    window->flex_track_align = track_align;
    ldWindowSetFlexAlign(ld_window,
                         s_flex_main_align_to_ld(main_align),
                         s_flex_cross_align_to_ld(cross_align));
    ldWindowSetFlexTrackAlign(ld_window, s_flex_track_align_to_ld(track_align));
    tinyui_window_sync_padding(window);
    return 0;
}

int tinyui_window_apply_flex_gap(struct tinyui_window *window, int item_gap, int track_gap)
{
    ldWindow_t *ld_window = tinyui_window_ld_of(window);

    if (window == 0 || ld_window == 0 || item_gap < 0 || track_gap < 0) {
        return -1;
    }

    window->flex_item_gap = item_gap;
    window->flex_track_gap = track_gap;
    ldWindowSetFlexGap(ld_window, (int16_t)item_gap, (int16_t)track_gap);
    tinyui_window_sync_padding(window);
    return 0;
}

int tinyui_window_apply_grid_columns(struct tinyui_window *window, const int *tracks, int count)
{
    ldWindow_t *ld_window = tinyui_window_ld_of(window);
    int window_tracks[TINYUI_LAYOUT_MAX_TRACKS];
    int i;

    if (window == 0 || ld_window == 0
        || s_copy_grid_tracks(window_tracks, window->backend_grid_cols, tracks, count) != 0) {
        return -1;
    }

    memcpy(window->grid_cols, window_tracks, sizeof(window->grid_cols));
    window->grid_col_count = count;

    {
        for (i = 0; i < TINYUI_LAYOUT_MAX_TRACKS; ++i) {
            window->backend_grid_rows[i] = s_grid_track_to_ld(window->grid_rows[i]);
        }
        ldWindowSetGridDscArray(ld_window,
                                window->backend_grid_cols,
                                window->grid_row_count > 0 ? window->backend_grid_rows : NULL);
    }
    tinyui_window_sync_padding(window);
    return 0;
}

int tinyui_window_apply_grid_rows(struct tinyui_window *window, const int *tracks, int count)
{
    ldWindow_t *ld_window = tinyui_window_ld_of(window);
    int window_tracks[TINYUI_LAYOUT_MAX_TRACKS];
    int i;

    if (window == 0 || ld_window == 0
        || s_copy_grid_tracks(window_tracks, window->backend_grid_rows, tracks, count) != 0) {
        return -1;
    }

    memcpy(window->grid_rows, window_tracks, sizeof(window->grid_rows));
    window->grid_row_count = count;

    {
        for (i = 0; i < TINYUI_LAYOUT_MAX_TRACKS; ++i) {
            window->backend_grid_cols[i] = s_grid_track_to_ld(window->grid_cols[i]);
        }
        ldWindowSetGridDscArray(ld_window,
                                window->grid_col_count > 0 ? window->backend_grid_cols : NULL,
                                window->backend_grid_rows);
    }
    tinyui_window_sync_padding(window);
    return 0;
}

int tinyui_window_apply_grid_gap(struct tinyui_window *window, int row_gap, int col_gap)
{
    ldWindow_t *ld_window = tinyui_window_ld_of(window);

    if (window == 0 || ld_window == 0 || row_gap < 0 || col_gap < 0) {
        return -1;
    }

    window->grid_row_gap = row_gap;
    window->grid_col_gap = col_gap;
    ldWindowSetGridGap(ld_window, (int16_t)row_gap, (int16_t)col_gap);
    tinyui_window_sync_padding(window);
    return 0;
}

int tinyui_window_apply_grid_align(struct tinyui_window *window,
                                   enum tinyui_align col_align,
                                   enum tinyui_align row_align)
{
    ldWindow_t *ld_window = tinyui_window_ld_of(window);

    if (window == 0 || ld_window == 0) {
        return -1;
    }

    window->grid_col_align = col_align;
    window->grid_row_align = row_align;
    ldWindowSetGridAlign(ld_window,
                         s_grid_align_to_ld(col_align),
                         s_grid_align_to_ld(row_align));
    tinyui_window_sync_padding(window);
    return 0;
}

static void tinyui_window_compute_display_root_size(struct tinyui_app *app, int16_t *width, int16_t *height)
{
    struct tinyui_display_config config = {0};

    if (width == 0 || height == 0) {
        return;
    }

    *width = LD_CFG_SCREEN_WIDTH;
    *height = LD_CFG_SCREEN_HEIGHT;
    if (app == 0) {
        return;
    }

    if (tinyui_display_get_config(app, &config) == 0 && config.width > 0 && config.height > 0) {
        *width = (int16_t)config.width;
        *height = (int16_t)config.height;
    }
}

/* C2: a window is "ok" when its folded backend binding is in place. */
static int tinyui_window_ok(struct tinyui_window *window)
{
    return window != 0
        && window->widget.ld_widget != 0
        && window->widget.owner != 0;
}

static int tinyui_window_props_ok(const struct tinyui_window_props *props)
{
    int has_padding_sentinel;
    int has_explicit_padding_group;

    has_padding_sentinel = props != 0
        && (props->padding_left == -1
            || props->padding_top == -1
            || props->padding_right == -1
            || props->padding_bottom == -1);
    has_explicit_padding_group = props != 0
        && props->padding_left >= 0
        && props->padding_top >= 0
        && props->padding_right >= 0
        && props->padding_bottom >= 0;

    return props != 0
        && props->id != 0
        && props->radius >= 0
        && props->padding >= 0
        && ((has_padding_sentinel && props->padding_left == -1
             && props->padding_top == -1
             && props->padding_right == -1
             && props->padding_bottom == -1)
            || has_explicit_padding_group);
}

/**
 * @brief Create window widget
 *
 * @param[in] app Application instance
 * @param[in] id Widget identifier string
 * @return Pointer to the object on success, NULL on failure
 */

struct tinyui_window *tinyui_window_create(struct tinyui_app *app, const char *id)
{
    struct tinyui_window *window;
    struct tinyui_app *app_state;
    ldWindow_t *ld_root;
    int16_t root_width;
    int16_t root_height;
    uint16_t name_id;

    if (app == 0 || id == 0) {
        return 0;
    }

    app_state = tinyui_runtime_bridge_backend_state(app);
    if (app_state == 0 || app_state->ld_scene == 0) {
        return 0;
    }

    tinyui_window_compute_display_root_size(app, &root_width, &root_height);

    /* Phantom scene root (nameId=0): created once, never exposed as a user
     * widget.  All user windows are children of this node, preventing the
     * "second window replaces first as scene root" collision. */
    if (app_state->ld_scene->ptNodeRoot == 0) {
        if (ldWindow_init(app_state->ld_scene, NULL, 0, 0, 0, 0,
                          root_width, root_height) == 0) {
            return 0;
        }
    }

    name_id = ++app_state->next_ld_name_id;
    ld_root = ldWindow_init(app_state->ld_scene, NULL, name_id, 0, 0, 0,
                            root_width, root_height);
    if (ld_root == 0) {
        return 0;
    }

    /* C2: single calloc — the wrapper struct is gone. */
    window = calloc(1, sizeof(*window));
    if (window == 0) {
        ldWindow_depose(app_state->ld_scene, ld_root);
        return 0;
    }

    tinyui_window_set_defaults(window, id);

    /* C2: fold the binding directly onto the widget struct (no host wrapper). */
    window->widget.ld_widget = ld_root;
    window->widget.ld_name_id = name_id;
    window->widget.kind = TINYUI_BACKEND_WIDGET_WINDOW;
    window->widget.owner = app_state;
    ((ldBase_t *)ld_root)->pInfo = &window->widget;

    if (tinyui_runtime_bridge_bind_leaf_widget(&window->widget, app_state) != 0) {
        ((ldBase_t *)ld_root)->pInfo = 0;
        window->widget.ld_widget = 0;
        ldWindow_depose(app_state->ld_scene, ld_root);
        free(window);
        return 0;
    }
    return window;
}

struct tinyui_window *tinyui_window_create_child(struct tinyui_window *parent, const char *id)
{
    struct tinyui_window *window;
    struct tinyui_app *app_state;
    ldWindow_t *ld_window;
    uint16_t name_id;
    uint16_t parent_name_id;

    if (parent == 0 || id == 0 || parent->widget.ld_widget == 0
        || parent->widget.owner == 0) {
        return 0;
    }

    app_state = tinyui_runtime_bridge_backend_state(parent->widget.owner);
    if (app_state == 0 || app_state->ld_scene == 0) {
        return 0;
    }

    parent_name_id = parent->widget.ld_name_id;
    name_id = ++app_state->next_ld_name_id;
    if (name_id == 0) {
        return 0;
    }

    ld_window = ldWindow_init(app_state->ld_scene,
                              NULL,
                              name_id,
                              parent_name_id,
                              0,
                              0,
                              160,
                              80);
    if (ld_window == 0) {
        return 0;
    }

    window = calloc(1, sizeof(*window));
    if (window == 0) {
        ldWindow_depose(app_state->ld_scene, ld_window);
        return 0;
    }

    tinyui_window_set_defaults(window, id);

    /* C2: fold the binding directly onto the widget struct (no host wrapper). */
    window->widget.ld_widget = ld_window;
    window->widget.ld_name_id = name_id;
    window->widget.kind = TINYUI_BACKEND_WIDGET_WINDOW;
    window->widget.owner = parent->widget.owner;
    ((ldBase_t *)ld_window)->pInfo = &window->widget;

    if (tinyui_runtime_bridge_bind_leaf_widget(&window->widget,
                                               parent->widget.owner) != 0) {
        ((ldBase_t *)ld_window)->pInfo = 0;
        window->widget.ld_widget = 0;
        ldWindow_depose(app_state->ld_scene, ld_window);
        free(window);
        return 0;
    }
    return window;
}

/**
 * @brief Create window widget with properties
 *
 * @param[in] app Application instance
 * @param[in] props Properties structure
 * @return Pointer to the object on success, NULL on failure
 */

struct tinyui_window *tinyui_window_create_with_props(struct tinyui_app *app,
                                                      const struct tinyui_window_props *props)
{
    struct tinyui_window *window;

    if (!tinyui_window_props_ok(props)) {
        return 0;
    }

    window = tinyui_window_create(app, props->id);
    if (window == 0) {
        return 0;
    }

    if (props->style_class != 0
        && tinyui_widget_set_style_class(&window->widget, props->style_class) != 0) {
        tinyui_window_do_free_internal(window);
        return 0;
    }
    if (tinyui_widget_set_user_data(&window->widget, props->user_data) != 0
        || tinyui_widget_set_bg_color(&window->widget, props->bg_color) != 0
        || tinyui_window_set_color(window, props->bg_color) != 0
        || tinyui_widget_set_text_color(&window->widget, props->text_color) != 0
        || tinyui_widget_set_border_color(&window->widget, props->border_color) != 0
        || tinyui_widget_set_radius(&window->widget, props->radius) != 0
        || tinyui_widget_set_padding(&window->widget, props->padding) != 0
        || tinyui_window_set_background_source(window, props->background_source) != 0
        || (props->padding_left != -1
            && tinyui_window_set_padding(window,
                                         props->padding_left,
                                         props->padding_top,
                                         props->padding_right,
                                         props->padding_bottom) != 0)) {
        tinyui_window_do_free_internal(window);
        return 0;
    }

    return window;
}

/**
 * @brief Set background source of window
 *
 * @param[in] window Window instance
 * @param[in] source Image source
 * @return -1 on failure
 */

int tinyui_window_set_background_source(struct tinyui_window *window,
                                        struct tinyui_image_source *source)
{
    ldWindow_t *ld_window;

    if (!tinyui_window_ok(window)) {
        return -1;
    }
    if (source != 0 && source->img_tile == 0) {
        return -1;
    }

    ld_window = tinyui_window_ld_of(window);
    if (ld_window == 0) {
        return -1;
    }

    ldWindowSetImage(ld_window,
                     source != 0 ? source->img_tile : 0,
                     source != 0 ? source->mask_tile : 0);
    return 0;
}

/**
 * @brief Set background offset of window
 *
 * @param[in] window Window instance
 * @param[in] offset_x Horizontal offset
 * @param[in] offset_y Vertical offset
 * @return 0 on success, -1 on failure
 */

int tinyui_window_set_background_offset(struct tinyui_window *window, int offset_x, int offset_y)
{
    struct tinyui_app *app_state;
    int16_t root_width = 0;
    int16_t root_height = 0;
    ldWindow_t *ld_window;
    int16_t bg_width;
    int16_t bg_height;

    if (!tinyui_window_ok(window)) {
        return -1;
    }

    if (window->widget.owner == 0) {
        return -1;
    }

    app_state = tinyui_runtime_bridge_backend_state(window->widget.owner);
    ld_window = tinyui_window_ld_of(window);
    if (app_state == 0 || app_state->ld_scene == 0 || ld_window == 0) {
        return -1;
    }

    tinyui_window_compute_display_root_size(window->widget.owner, &root_width, &root_height);
    bg_width = ld_window->use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tSize.iWidth;
    bg_height = ld_window->use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tSize.iHeight;
    if (bg_width <= 0) {
        bg_width = root_width;
    }
    if (bg_height <= 0) {
        bg_height = root_height;
    }
    if (bg_width < root_width) {
        bg_width = root_width;
    }
    if (bg_height < root_height) {
        bg_height = root_height;
    }

    ldBaseBgMove(app_state->ld_scene, bg_width, bg_height, (int16_t)offset_x, (int16_t)offset_y);

    /* Set window position and size.  With a phantom scene root the user window
     * is a child of the phantom, so we must also sync its size to the extended
     * scene size that ldBaseBgMove computed on the phantom. */
    if (ld_window != (ldWindow_t *)app_state->ld_scene->ptNodeRoot) {
        ldBase_t *phantom = (ldBase_t *)app_state->ld_scene->ptNodeRoot;
        int16_t ext_w = phantom != 0
            ? phantom->use_as__arm_2d_control_node_t.tRegion.tSize.iWidth
            : bg_width;
        int16_t ext_h = phantom != 0
            ? phantom->use_as__arm_2d_control_node_t.tRegion.tSize.iHeight
            : bg_height;
        ldBaseSetX((ldBase_t *)ld_window, (int16_t)offset_x);
        ldBaseSetY((ldBase_t *)ld_window, (int16_t)offset_y);
        ldBaseSetWidth((ldBase_t *)ld_window, ext_w);
        ldBaseSetHeight((ldBase_t *)ld_window, ext_h);
        bg_width = ext_w;
        bg_height = ext_h;
    } else {
        if (ld_window->use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tSize.iWidth < bg_width) {
            ldBaseSetWidth((ldBase_t *)ld_window, bg_width);
        }
        if (ld_window->use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tSize.iHeight < bg_height) {
            ldBaseSetHeight((ldBase_t *)ld_window, bg_height);
        }
        ldBaseSetX((ldBase_t *)ld_window, (int16_t)offset_x);
        ldBaseSetY((ldBase_t *)ld_window, (int16_t)offset_y);
    }

    if (ld_window->use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tSize.iWidth < bg_width
        || ld_window->use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tSize.iHeight < bg_height) {
        return -1;
    }

    window->background_offset_x = offset_x;
    window->background_offset_y = offset_y;
    return 0;
}

/**
 * @brief Get background offset of window
 *
 * @param[out] window Window instance
 * @param[in] offset_x Horizontal offset
 * @param[in] offset_y Vertical offset
 * @return 0 on success, -1 on failure
 */

int tinyui_window_get_background_offset(struct tinyui_window *window,
                                        int *offset_x,
                                        int *offset_y)
{
    if (!tinyui_window_ok(window) || offset_x == 0 || offset_y == 0) {
        return -1;
    }

    *offset_x = window->background_offset_x;
    *offset_y = window->background_offset_y;
    return 0;
}

/**
 * @brief Set color of window
 *
 * @param[in] window Window instance
 * @param[in] rgb RGB color value (0xRRGGBB)
 * @return -1 on failure
 */

int tinyui_window_set_color(struct tinyui_window *window, unsigned int rgb)
{
    ldWindow_t *ld_window;

    if (!tinyui_window_ok(window) || rgb > 0xFFFFFFU) {
        return -1;
    }

    if (s_window_set_bg_color_fail_flag != 0) {
        s_window_set_bg_color_fail_flag = 0;
        return -1;
    }

    ld_window = tinyui_window_ld_of(window);
    if (ld_window == 0) {
        return -1;
    }

    window->widget.bg_color = rgb;
    ldWindowSetColor(ld_window, (ldColor)tinyui_rgb_to_ld_color(rgb));
    return 0;
}

/**
 * @brief Get color of window
 *
 * @param[out] window Window instance
 * @param[in] rgb RGB color value (0xRRGGBB)
 * @return -1 on failure
 */

int tinyui_window_get_color(struct tinyui_window *window, unsigned int *rgb)
{
    ldWindow_t *ld_window;

    if (!tinyui_window_ok(window) || rgb == 0) {
        return -1;
    }

    ld_window = tinyui_window_ld_of(window);
    if (ld_window == 0) {
        return -1;
    }

    *rgb = s_ld_color_to_rgb_legacy((unsigned int)ldWindowGetColor(ld_window));
    return 0;
}

/**
 * @brief Set layout type of window
 *
 * @param[in] window Window instance
 * @param[in] type Type
 * @return -1 on failure
 */

int tinyui_window_set_layout_type(struct tinyui_window *window,
                                  enum tinyui_window_layout_type type)
{
    if (!tinyui_window_ok(window)
        || (type != TINYUI_WINDOW_LAYOUT_NONE
            && type != TINYUI_WINDOW_LAYOUT_FLEX
            && type != TINYUI_WINDOW_LAYOUT_GRID)) {
        return -1;
    }

    return tinyui_window_do_set_layout_type(window, type);
}

/**
 * @brief Set padding of window
 *
 * @param[in] window Window instance
 * @param[in] left Left padding
 * @param[in] top Top padding
 * @param[in] right Right padding
 * @param[in] bottom Bottom padding
 * @return -1 on failure
 */

int tinyui_window_set_padding(struct tinyui_window *window,
                              int left,
                              int top,
                              int right,
                              int bottom)
{
    if (!tinyui_window_ok(window)
        || left < 0
        || top < 0
        || right < 0
        || bottom < 0) {
        return -1;
    }

    return tinyui_window_apply_padding_group(window, left, top, right, bottom);
}

/**
 * @brief Set grid padding of window
 *
 * @param[in] window Window instance
 * @param[in] left Left padding
 * @param[in] top Top padding
 * @param[in] right Right padding
 * @param[in] bottom Bottom padding
 * @return -1 on failure
 */

int tinyui_window_set_grid_padding(struct tinyui_window *window,
                                   int left,
                                   int top,
                                   int right,
                                   int bottom)
{
    if (!tinyui_window_ok(window)
        || left < 0
        || top < 0
        || right < 0
        || bottom < 0) {
        return -1;
    }

    return tinyui_window_apply_explicit_grid_padding(window, left, top, right, bottom);
}

/**
 * @brief Set gap of window
 *
 * @param[in] window Window instance
 * @param[in] gap Gap in pixels
 * @return 0 on success, -1 on failure
 */

int tinyui_window_set_gap(struct tinyui_window *window, int gap)
{
    if (!tinyui_window_ok(window) || gap < 0) {
        return -1;
    }

    return tinyui_window_do_set_gap(window, gap);
}


/**
 * @brief Get padding group of window
 *
 * @param[in] window Window instance
 * @param[out] left Left padding
 * @param[out] top Top padding
 * @param[out] right Right padding
 * @param[out] bottom Bottom padding
 * @return 0 on success, -1 on failure
 */

int tinyui_window_get_padding_group(struct tinyui_window *window,
                                    int *left,
                                    int *top,
                                    int *right,
                                    int *bottom)
{
    ldWindow_t *ld_window;

    if (!tinyui_window_ok(window)
        || left == 0
        || top == 0
        || right == 0
        || bottom == 0) {
        return -1;
    }

    ld_window = tinyui_window_ld_of(window);
    if (ld_window == 0 || ld_window->pLayoutPaddingGroup == 0) {
        return -1;
    }
    *left = ld_window->pLayoutPaddingGroup->left;
    *top = ld_window->pLayoutPaddingGroup->top;
    *right = ld_window->pLayoutPaddingGroup->right;
    *bottom = ld_window->pLayoutPaddingGroup->bottom;
    return 0;
}

void *tinyui_window_get_backend_widget(struct tinyui_window *window)
{
    if (window == 0) {
        return 0;
    }
    /* C3-T4: the wrapper is gone — expose the folded widget directly. */
    return &window->widget;
}
