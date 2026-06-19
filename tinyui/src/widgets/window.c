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

struct tinyui_image_source;

static int tinyui_window_fail_next_set_bg_color = 0;

#define TINYUI_WINDOW_LAYOUT_MAX_TRACKS TINYUI_BACKEND_LAYOUT_MAX_TRACKS

static ldColor tinyui_window_rgb_to_ld_color(unsigned int rgb)
{
    return __RGB((rgb >> 16) & 0xFFU, (rgb >> 8) & 0xFFU, rgb & 0xFFU);
}

static unsigned int tinyui_window_ld_color_to_rgb(ldColor color)
{
    uint32_t red = ((uint32_t)color >> 11) & 0x1FU;
    uint32_t green = ((uint32_t)color >> 5) & 0x3FU;
    uint32_t blue = (uint32_t)color & 0x1FU;

    red = (red * 255U) / 31U;
    green = (green * 255U) / 63U;
    blue = (blue * 255U) / 31U;
    return (red << 16) | (green << 8) | blue;
}

static ldFlexFlow_t tinyui_window_map_flex_flow(enum tinyui_flex_flow flow)
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

static ldFlexMainAlign_t tinyui_window_map_flex_main_align(enum tinyui_align align)
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

static ldFlexCrossAlign_t tinyui_window_map_flex_cross_align(enum tinyui_align align)
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

static ldFlexTrackAlign_t tinyui_window_map_flex_track_align(enum tinyui_align align)
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

static ldGridAlign_t tinyui_window_map_grid_align(enum tinyui_align align)
{
    return (ldGridAlign_t)tinyui_native_align_to_ld_grid((enum tinyui_native_align)align);
}

static int16_t tinyui_window_map_grid_track(int value)
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

static int tinyui_window_copy_grid_tracks(int *dst, int16_t *backend_dst, const int *src, int count)
{
    int i;

    if (dst == 0 || backend_dst == 0 || src == 0 || count <= 0 || count > TINYUI_LAYOUT_MAX_TRACKS) {
        return -1;
    }

    for (i = 0; i < count; ++i) {
        dst[i] = src[i];
        backend_dst[i] = tinyui_window_map_grid_track(src[i]);
    }
    for (; i < TINYUI_LAYOUT_MAX_TRACKS; ++i) {
        dst[i] = 0;
        backend_dst[i] = LD_GRID_TEMPLATE_LAST;
    }
    return 0;
}

struct tinyui_window_backend_host {
    struct tinyui_backend_widget widget;
    ldPadding_t padding_group;
    int has_padding_group;
};

int tinyui_runtime_bridge_unbind_host(void *backend_widget);
int tinyui_runtime_bridge_detach_from_parent(void *backend_widget);

static struct tinyui_backend_widget *tinyui_window_get_backend(struct tinyui_window *window)
{
    if (window == 0) {
        return 0;
    }

    return (struct tinyui_backend_widget *)window->widget.backend_widget;
}

static struct tinyui_window_backend_host *tinyui_window_get_backend_host(struct tinyui_window *window)
{
    return (struct tinyui_window_backend_host *)tinyui_window_get_backend(window);
}

static void tinyui_window_apply_padding_contract(struct tinyui_window *window);

static int tinyui_window_apply_layout_type_impl(struct tinyui_window *window,
                                                enum tinyui_window_layout_type type);

static int tinyui_window_apply_flex_contract_impl(struct tinyui_window *window,
                                                  enum tinyui_flex_flow flow,
                                                  enum tinyui_align main_align,
                                                  enum tinyui_align cross_align,
                                                  enum tinyui_align track_align,
                                                  int item_gap,
                                                  int track_gap);

static int tinyui_window_apply_generic_gap_impl(struct tinyui_window *window, int gap);

static ldWindow_t *tinyui_window_get_ld_window(struct tinyui_window *window)
{
    struct tinyui_backend_widget *backend = tinyui_window_get_backend(window);
    ldBase_t *ld_base;

    if (backend == 0 || backend->ld_widget == 0) {
        return 0;
    }

    if (backend->kind != TINYUI_BACKEND_WIDGET_WINDOW &&
        backend->kind != TINYUI_BACKEND_WIDGET_BACKGROUND) {
        return 0;
    }

    ld_base = (ldBase_t *)backend->ld_widget;
    if (ld_base->widgetType != widgetTypeWindow &&
        ld_base->widgetType != widgetTypeBackground) {
        return 0;
    }

    return (ldWindow_t *)ld_base;
}

static ldLayoutType_t tinyui_window_map_layout_type(enum tinyui_window_layout_type type)
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
    struct tinyui_backend_widget *backend = tinyui_window_get_backend(window);
    ldWindow_t *ld_window = tinyui_window_get_ld_window(window);

    if (window == 0 || backend == 0 || ld_window == 0 || padding < 0) {
        return -1;
    }

    backend->window_layout.padding = padding;
    backend->window_layout.padding_left = padding;
    backend->window_layout.padding_top = padding;
    backend->window_layout.padding_right = padding;
    backend->window_layout.padding_bottom = padding;
    backend->window_layout.has_explicit_flex_padding = 0;
    backend->window_layout.grid_padding_left = padding;
    backend->window_layout.grid_padding_top = padding;
    backend->window_layout.grid_padding_right = padding;
    backend->window_layout.grid_padding_bottom = padding;
    backend->window_layout.has_explicit_grid_padding = 0;
    tinyui_window_apply_padding_contract(window);
    return 0;
}

int tinyui_window_apply_explicit_padding(struct tinyui_window *window,
                                         int left,
                                         int top,
                                         int right,
                                         int bottom)
{
    struct tinyui_backend_widget *backend = tinyui_window_get_backend(window);
    ldWindow_t *ld_window = tinyui_window_get_ld_window(window);
    ldLayoutType_t layout_type;

    if (window == 0 || backend == 0 || ld_window == 0
        || left < 0 || top < 0 || right < 0 || bottom < 0) {
        return -1;
    }

    backend->window_layout.padding_left = left;
    backend->window_layout.padding_top = top;
    backend->window_layout.padding_right = right;
    backend->window_layout.padding_bottom = bottom;
    backend->window_layout.has_explicit_flex_padding = 1;
    layout_type = ld_window->layoutTpye;
    ldWindowSetPadding(ld_window, (ldPadding_t){
        .left = (int16_t)left,
        .top = (int16_t)top,
        .right = (int16_t)right,
        .bottom = (int16_t)bottom,
    });
    ld_window->layoutTpye = layout_type;
    return 0;
}

int tinyui_window_apply_explicit_grid_padding(struct tinyui_window *window,
                                              int left,
                                              int top,
                                              int right,
                                              int bottom)
{
    struct tinyui_backend_widget *backend = tinyui_window_get_backend(window);
    ldWindow_t *ld_window = tinyui_window_get_ld_window(window);
    ldLayoutType_t layout_type;

    if (window == 0 || backend == 0 || ld_window == 0
        || left < 0 || top < 0 || right < 0 || bottom < 0) {
        return -1;
    }

    backend->window_layout.grid_padding_left = left;
    backend->window_layout.grid_padding_top = top;
    backend->window_layout.grid_padding_right = right;
    backend->window_layout.grid_padding_bottom = bottom;
    backend->window_layout.has_explicit_grid_padding = 1;
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

static void tinyui_window_init_defaults(struct tinyui_window *window, const char *id, void *backend_widget)
{
    window->id = id;
    window->widget.backend_widget = backend_widget;
    window->widget.visible = 1;
    window->widget.enabled = 1;
    window->flex_flow = TINYUI_FLEX_FLOW_ROW;
    window->flex_main_align = TINYUI_ALIGN_START;
    window->flex_cross_align = TINYUI_ALIGN_START;
    window->flex_track_align = TINYUI_ALIGN_START;
    window->grid_col_align = TINYUI_ALIGN_START;
    window->grid_row_align = TINYUI_ALIGN_START;
}

static void tinyui_window_dispose_partial(struct tinyui_window *window)
{
    struct tinyui_backend_widget *backend;
    struct tinyui_app *app_state;
    int clears_scene_root = 0;

    if (window == 0 || window->widget.backend_widget == 0) {
        free(window);
        return;
    }

    backend = (struct tinyui_backend_widget *)window->widget.backend_widget;
    app_state = backend->owner != 0
        ? tinyui_runtime_bridge_backend_state(backend->owner)
        : 0;

    (void)tinyui_runtime_bridge_unbind_host(backend);
    if (backend->parent != 0) {
        (void)tinyui_runtime_bridge_detach_from_parent(backend);
    }
    if (backend->parent == 0
        && app_state != 0
        && app_state->ld_scene != 0
        && app_state->ld_scene->ptNodeRoot == (arm_2d_control_node_t *)backend->ld_widget) {
        clears_scene_root = 1;
    }
    if (app_state != 0 && app_state->ld_scene != 0 && backend->ld_widget != 0) {
        ldWindow_depose(app_state->ld_scene, (ldWindow_t *)backend->ld_widget);
        if (clears_scene_root) {
            app_state->ld_scene->ptNodeRoot = 0;
        }
    }

    free(backend);
    free(window);
}

void tinyui_window_test_fail_next_set_bg_color(void)
{
    tinyui_window_fail_next_set_bg_color = 1;
}

static int tinyui_window_set_padding_group_impl(struct tinyui_window *window,
                                                int left,
                                                int top,
                                                int right,
                                                int bottom)
{
    struct tinyui_window_backend_host *host = tinyui_window_get_backend_host(window);

    if (host == 0) {
        return -1;
    }

    if (tinyui_window_get_ld_window(window) == 0) {
        return -1;
    }

    host->padding_group.left = (int16_t)left;
    host->padding_group.top = (int16_t)top;
    host->padding_group.right = (int16_t)right;
    host->padding_group.bottom = (int16_t)bottom;
    host->has_padding_group = 1;
    tinyui_window_apply_padding_contract(window);
    return 0;
}

static void tinyui_window_apply_padding_contract(struct tinyui_window *window)
{
    struct tinyui_window_backend_host *host = tinyui_window_get_backend_host(window);
    struct tinyui_backend_widget *backend = tinyui_window_get_backend(window);
    ldLayoutType_t layout_type;
    ldPadding_t flex_padding;
    ldPadding_t grid_padding;
    ldWindow_t *ld_window;

    if (window == 0 || host == 0 || backend == 0) {
        return;
    }

    ld_window = tinyui_window_get_ld_window(window);
    if (ld_window == 0) {
        return;
    }

    flex_padding = (ldPadding_t){
        .left = (int16_t)backend->window_layout.padding_left,
        .top = (int16_t)backend->window_layout.padding_top,
        .right = (int16_t)backend->window_layout.padding_right,
        .bottom = (int16_t)backend->window_layout.padding_bottom,
    };
    grid_padding = (ldPadding_t){
        .left = (int16_t)backend->window_layout.grid_padding_left,
        .top = (int16_t)backend->window_layout.grid_padding_top,
        .right = (int16_t)backend->window_layout.grid_padding_right,
        .bottom = (int16_t)backend->window_layout.grid_padding_bottom,
    };

    layout_type = ld_window->layoutTpye;
    if (host->has_padding_group) {
        flex_padding = host->padding_group;
        grid_padding = host->padding_group;
    }
    ldWindowSetPadding(ld_window, flex_padding);
    ldWindowSetGridPadding(ld_window, grid_padding);
    if (host->has_padding_group) {
        ldWindowSetPaddingGroup(ld_window, &host->padding_group);
    }
    ld_window->layoutTpye = layout_type;
}

static int tinyui_window_apply_layout_type_impl(struct tinyui_window *window,
                                                enum tinyui_window_layout_type type)
{
    ldWindow_t *ld_window = tinyui_window_get_ld_window(window);

    if (window == 0 || ld_window == 0
        || (type != TINYUI_WINDOW_LAYOUT_NONE
            && type != TINYUI_WINDOW_LAYOUT_FLEX
            && type != TINYUI_WINDOW_LAYOUT_GRID)) {
        return -1;
    }

    ldWindowSetLayout(ld_window, tinyui_window_map_layout_type(type));
    tinyui_window_apply_padding_contract(window);
    return 0;
}

static int tinyui_window_apply_flex_contract_impl(struct tinyui_window *window,
                                                  enum tinyui_flex_flow flow,
                                                  enum tinyui_align main_align,
                                                  enum tinyui_align cross_align,
                                                  enum tinyui_align track_align,
                                                  int item_gap,
                                                  int track_gap)
{
    struct tinyui_backend_widget *backend = tinyui_window_get_backend(window);
    ldWindow_t *ld_window = tinyui_window_get_ld_window(window);

    if (window == 0 || backend == 0 || ld_window == 0 || item_gap < 0 || track_gap < 0) {
        return -1;
    }

    window->flex_flow = flow;
    window->flex_main_align = main_align;
    window->flex_cross_align = cross_align;
    window->flex_track_align = track_align;
    window->flex_item_gap = item_gap;
    window->flex_track_gap = track_gap;

    backend->window_layout.flex_flow = flow;
    backend->window_layout.flex_main_align = main_align;
    backend->window_layout.flex_cross_align = cross_align;
    backend->window_layout.flex_track_align = track_align;
    backend->window_layout.flex_item_gap = item_gap;
    backend->window_layout.flex_track_gap = track_gap;

    ldWindowSetFlexFlow(ld_window, tinyui_window_map_flex_flow(flow));
    ldWindowSetFlexAlign(ld_window,
                         tinyui_window_map_flex_main_align(main_align),
                         tinyui_window_map_flex_cross_align(cross_align));
    ldWindowSetFlexTrackAlign(ld_window, tinyui_window_map_flex_track_align(track_align));
    ldWindowSetFlexGap(ld_window, (int16_t)item_gap, (int16_t)track_gap);
    tinyui_window_apply_padding_contract(window);
    return 0;
}

static int tinyui_window_apply_generic_gap_impl(struct tinyui_window *window, int gap)
{
    struct tinyui_backend_widget *backend = tinyui_window_get_backend(window);
    ldWindow_t *ld_window = tinyui_window_get_ld_window(window);
    ldLayoutType_t layout_type;

    if (window == 0 || backend == 0 || ld_window == 0 || gap < 0) {
        return -1;
    }

    window->flex_item_gap = gap;
    window->flex_track_gap = gap;
    backend->window_layout.flex_item_gap = gap;
    backend->window_layout.flex_track_gap = gap;
    layout_type = ld_window->layoutTpye;
    ldWindowSetGap(ld_window, (int16_t)gap);
    ld_window->layoutTpye = layout_type;
    return 0;
}

int tinyui_window_apply_flex_flow(struct tinyui_window *window, enum tinyui_flex_flow flow)
{
    ldWindow_t *ld_window = tinyui_window_get_ld_window(window);

    if (window == 0 || ld_window == 0) {
        return -1;
    }

    window->flex_flow = flow;
    ldWindowSetFlexFlow(ld_window, tinyui_window_map_flex_flow(flow));
    tinyui_window_apply_padding_contract(window);
    return 0;
}

int tinyui_window_apply_flex_align(struct tinyui_window *window,
                                   enum tinyui_align main_align,
                                   enum tinyui_align cross_align,
                                   enum tinyui_align track_align)
{
    ldWindow_t *ld_window = tinyui_window_get_ld_window(window);

    if (window == 0 || ld_window == 0) {
        return -1;
    }

    window->flex_main_align = main_align;
    window->flex_cross_align = cross_align;
    window->flex_track_align = track_align;
    ldWindowSetFlexAlign(ld_window,
                         tinyui_window_map_flex_main_align(main_align),
                         tinyui_window_map_flex_cross_align(cross_align));
    ldWindowSetFlexTrackAlign(ld_window, tinyui_window_map_flex_track_align(track_align));
    tinyui_window_apply_padding_contract(window);
    return 0;
}

int tinyui_window_apply_flex_gap(struct tinyui_window *window, int item_gap, int track_gap)
{
    struct tinyui_backend_widget *backend = tinyui_window_get_backend(window);
    ldWindow_t *ld_window = tinyui_window_get_ld_window(window);

    if (window == 0 || backend == 0 || ld_window == 0 || item_gap < 0 || track_gap < 0) {
        return -1;
    }

    window->flex_item_gap = item_gap;
    window->flex_track_gap = track_gap;
    backend->window_layout.flex_item_gap = item_gap;
    backend->window_layout.flex_track_gap = track_gap;
    ldWindowSetFlexGap(ld_window, (int16_t)item_gap, (int16_t)track_gap);
    tinyui_window_apply_padding_contract(window);
    return 0;
}

int tinyui_window_apply_grid_columns(struct tinyui_window *window, const int *tracks, int count)
{
    struct tinyui_backend_widget *backend = tinyui_window_get_backend(window);
    ldWindow_t *ld_window = tinyui_window_get_ld_window(window);
    int window_tracks[TINYUI_LAYOUT_MAX_TRACKS];
    int16_t backend_tracks[TINYUI_LAYOUT_MAX_TRACKS];

    if (window == 0 || backend == 0 || ld_window == 0
        || tinyui_window_copy_grid_tracks(window_tracks, backend_tracks, tracks, count) != 0) {
        return -1;
    }

    memcpy(window->grid_cols, window_tracks, sizeof(window->grid_cols));
    memcpy(backend->window_layout.grid_cols, backend_tracks, sizeof(backend->window_layout.grid_cols));
    window->grid_col_count = count;
    backend->window_layout.grid_col_count = count;
    ldWindowSetGridDscArray(ld_window,
                            backend->window_layout.grid_cols,
                            backend->window_layout.grid_row_count > 0
                                ? backend->window_layout.grid_rows
                                : NULL);
    tinyui_window_apply_padding_contract(window);
    return 0;
}

int tinyui_window_apply_grid_rows(struct tinyui_window *window, const int *tracks, int count)
{
    struct tinyui_backend_widget *backend = tinyui_window_get_backend(window);
    ldWindow_t *ld_window = tinyui_window_get_ld_window(window);
    int window_tracks[TINYUI_LAYOUT_MAX_TRACKS];
    int16_t backend_tracks[TINYUI_LAYOUT_MAX_TRACKS];

    if (window == 0 || backend == 0 || ld_window == 0
        || tinyui_window_copy_grid_tracks(window_tracks, backend_tracks, tracks, count) != 0) {
        return -1;
    }

    memcpy(window->grid_rows, window_tracks, sizeof(window->grid_rows));
    memcpy(backend->window_layout.grid_rows, backend_tracks, sizeof(backend->window_layout.grid_rows));
    window->grid_row_count = count;
    backend->window_layout.grid_row_count = count;
    ldWindowSetGridDscArray(ld_window,
                            backend->window_layout.grid_col_count > 0
                                ? backend->window_layout.grid_cols
                                : NULL,
                            backend->window_layout.grid_rows);
    tinyui_window_apply_padding_contract(window);
    return 0;
}

int tinyui_window_apply_grid_gap(struct tinyui_window *window, int row_gap, int col_gap)
{
    struct tinyui_backend_widget *backend = tinyui_window_get_backend(window);
    ldWindow_t *ld_window = tinyui_window_get_ld_window(window);

    if (window == 0 || backend == 0 || ld_window == 0 || row_gap < 0 || col_gap < 0) {
        return -1;
    }

    window->grid_row_gap = row_gap;
    window->grid_col_gap = col_gap;
    backend->window_layout.grid_row_gap = row_gap;
    backend->window_layout.grid_col_gap = col_gap;
    ldWindowSetGridGap(ld_window, (int16_t)row_gap, (int16_t)col_gap);
    tinyui_window_apply_padding_contract(window);
    return 0;
}

int tinyui_window_apply_grid_align(struct tinyui_window *window,
                                   enum tinyui_align col_align,
                                   enum tinyui_align row_align)
{
    struct tinyui_backend_widget *backend = tinyui_window_get_backend(window);
    ldWindow_t *ld_window = tinyui_window_get_ld_window(window);

    if (window == 0 || backend == 0 || ld_window == 0) {
        return -1;
    }

    window->grid_col_align = col_align;
    window->grid_row_align = row_align;
    backend->window_layout.grid_col_align = col_align;
    backend->window_layout.grid_row_align = row_align;
    ldWindowSetGridAlign(ld_window,
                         tinyui_window_map_grid_align(col_align),
                         tinyui_window_map_grid_align(row_align));
    tinyui_window_apply_padding_contract(window);
    return 0;
}

static void tinyui_window_get_root_size(struct tinyui_app *app, int16_t *width, int16_t *height)
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

static int tinyui_window_is_valid(struct tinyui_window *window)
{
    return window != 0 && window->widget.backend_widget != 0;
}

static int tinyui_window_props_are_valid(const struct tinyui_window_props *props)
{
    return props != 0
        && props->id != 0
        && props->radius >= 0
        && props->padding >= 0
        && props->padding_left >= 0
        && props->padding_top >= 0
        && props->padding_right >= 0
        && props->padding_bottom >= 0;
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
    struct tinyui_window_backend_host *host;
    struct tinyui_app *app_state;
    ldWindow_t *ld_root;
    int16_t root_width;
    int16_t root_height;

    if (app == 0 || id == 0) {
        return 0;
    }

    app_state = tinyui_runtime_bridge_backend_state(app);
    if (app_state == 0 || app_state->ld_scene == 0) {
        return 0;
    }

    host = calloc(1, sizeof(*host));
    if (host == 0) {
        return 0;
    }

    tinyui_window_get_root_size(app, &root_width, &root_height);
    ld_root = ldWindow_init(app_state->ld_scene, NULL, 0, 0, 0, 0, root_width, root_height);
    if (ld_root == 0) {
        free(host);
        return 0;
    }

    if (tinyui_widget_init_root(&host->widget,
                                        app,
                                        TINYUI_BACKEND_WIDGET_WINDOW,
                                        id,
                                        app->theme) != 0) {
        ldWindow_depose(app_state->ld_scene, ld_root);
        free(host);
        return 0;
    }
    host->widget.ld_widget = ld_root;
    host->widget.ld_name_id = 0;

    window = calloc(1, sizeof(*window));
    if (window == 0) {
        ldWindow_depose(app_state->ld_scene, ld_root);
        free(host);
        return 0;
    }

    tinyui_window_init_defaults(window, id, &host->widget);
    if (tinyui_runtime_bridge_bind_host(window->widget.backend_widget, &window->widget) != 0) {
        ldWindow_depose(app_state->ld_scene, ld_root);
        free(host);
        free(window);
        return 0;
    }
    return window;
}

struct tinyui_window *tinyui_window_create_child(struct tinyui_window *parent, const char *id)
{
    struct tinyui_window *window;
    struct tinyui_window_backend_host *host;
    struct tinyui_backend_widget *parent_backend;
    struct tinyui_app *app_state;
    ldWindow_t *ld_window;
    uint16_t name_id;

    if (parent == 0 || id == 0 || parent->widget.backend_widget == 0) {
        return 0;
    }

    parent_backend = (struct tinyui_backend_widget *)parent->widget.backend_widget;
    app_state = tinyui_runtime_bridge_backend_state_from_parent(parent_backend);
    if (app_state == 0 || app_state->ld_scene == 0 || parent_backend->ld_widget == 0) {
        return 0;
    }

    host = calloc(1, sizeof(*host));
    if (host == 0) {
        return 0;
    }

    name_id = tinyui_runtime_bridge_next_name_id(parent_backend);
    if (name_id == 0) {
        free(host);
        return 0;
    }

    ld_window = ldWindow_init(app_state->ld_scene,
                              NULL,
                              name_id,
                              parent_backend->ld_name_id,
                              0,
                              0,
                              160,
                              80);
    if (ld_window == 0) {
        free(host);
        return 0;
    }

    if (tinyui_widget_init_child(&host->widget,
                                         parent_backend,
                                         TINYUI_BACKEND_WIDGET_WINDOW,
                                         id,
                                         parent_backend->theme) != 0) {
        ldWindow_depose(app_state->ld_scene, ld_window);
        free(host);
        return 0;
    }
    host->widget.ld_widget = ld_window;
    host->widget.ld_name_id = name_id;
    if (tinyui_widget_attach_child(parent_backend, &host->widget) != 0) {
        ldWindow_depose(app_state->ld_scene, ld_window);
        free(host);
        return 0;
    }

    window = calloc(1, sizeof(*window));
    if (window == 0) {
        ldWindow_depose(app_state->ld_scene, ld_window);
        free(host);
        return 0;
    }

    tinyui_window_init_defaults(window, id, &host->widget);
    if (tinyui_runtime_bridge_bind_host(window->widget.backend_widget, &window->widget) != 0) {
        (void)tinyui_runtime_bridge_detach_from_parent(&host->widget);
        ldWindow_depose(app_state->ld_scene, ld_window);
        free(host);
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

    if (!tinyui_window_props_are_valid(props)) {
        return 0;
    }

    window = tinyui_window_create(app, props->id);
    if (window == 0) {
        return 0;
    }

    if (props->style_class != 0
        && tinyui_widget_set_style_class(&window->widget, props->style_class) != 0) {
        tinyui_window_dispose_partial(window);
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
        || (props->has_padding_group != 0
            && tinyui_window_set_padding_group(window,
                                               props->padding_left,
                                               props->padding_top,
                                               props->padding_right,
                                               props->padding_bottom) != 0)) {
        tinyui_window_dispose_partial(window);
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

    if (!tinyui_window_is_valid(window)) {
        return -1;
    }
    if (source != 0 && source->img_tile == 0) {
        return -1;
    }

    ld_window = tinyui_window_get_ld_window(window);
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
    struct tinyui_backend_widget *backend;
    struct tinyui_app *app_state;
    int16_t root_width = 0;
    int16_t root_height = 0;
    ldWindow_t *ld_window;
    int16_t bg_width;
    int16_t bg_height;

    if (!tinyui_window_is_valid(window)) {
        return -1;
    }

    backend = tinyui_window_get_backend(window);
    if (backend == 0 || backend->owner == 0) {
        return -1;
    }

    app_state = tinyui_runtime_bridge_backend_state(backend->owner);
    ld_window = tinyui_window_get_ld_window(window);
    if (app_state == 0 || app_state->ld_scene == 0 || ld_window == 0) {
        return -1;
    }

    tinyui_window_get_root_size(backend->owner, &root_width, &root_height);
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
    if (ld_window != (ldWindow_t *)app_state->ld_scene->ptNodeRoot) {
        ldBaseSetX((ldBase_t *)ld_window, (int16_t)offset_x);
        ldBaseSetY((ldBase_t *)ld_window, (int16_t)offset_y);
    }

    if (ld_window->use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tSize.iWidth < bg_width) {
        ldBaseSetWidth((ldBase_t *)ld_window, bg_width);
    }
    if (ld_window->use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tSize.iHeight < bg_height) {
        ldBaseSetHeight((ldBase_t *)ld_window, bg_height);
    }

    if (ld_window == (ldWindow_t *)app_state->ld_scene->ptNodeRoot) {
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
    if (!tinyui_window_is_valid(window) || offset_x == 0 || offset_y == 0) {
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

    if (!tinyui_window_is_valid(window) || rgb > 0xFFFFFFU) {
        return -1;
    }

    if (tinyui_window_fail_next_set_bg_color != 0) {
        tinyui_window_fail_next_set_bg_color = 0;
        return -1;
    }

    ld_window = tinyui_window_get_ld_window(window);
    if (ld_window == 0) {
        return -1;
    }

    window->widget.bg_color = rgb;
    ldWindowSetColor(ld_window, tinyui_window_rgb_to_ld_color(rgb));
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

    if (!tinyui_window_is_valid(window) || rgb == 0) {
        return -1;
    }

    ld_window = tinyui_window_get_ld_window(window);
    if (ld_window == 0) {
        return -1;
    }

    *rgb = tinyui_window_ld_color_to_rgb(ldWindowGetColor(ld_window));
    return 0;
}

/**
 * @brief Set padding group of window
 *
 * @param[in] window Window instance
 * @param[in] left Left padding
 * @param[in] top Top padding
 * @param[in] right Right padding
 * @param[in] bottom Bottom padding
 * @return -1 on failure
 */

int tinyui_window_set_padding_group(struct tinyui_window *window,
                                    int left,
                                    int top,
                                    int right,
                                    int bottom)
{
    if (!tinyui_window_is_valid(window)
        || left < 0
        || top < 0
        || right < 0
        || bottom < 0) {
        return -1;
    }

    return tinyui_window_set_padding_group_impl(window, left, top, right, bottom);
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
    if (!tinyui_window_is_valid(window)
        || (type != TINYUI_WINDOW_LAYOUT_NONE
            && type != TINYUI_WINDOW_LAYOUT_FLEX
            && type != TINYUI_WINDOW_LAYOUT_GRID)) {
        return -1;
    }

    return tinyui_window_apply_layout_type_impl(window, type);
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
    if (!tinyui_window_is_valid(window)
        || left < 0
        || top < 0
        || right < 0
        || bottom < 0) {
        return -1;
    }

    return tinyui_window_apply_explicit_padding(window, left, top, right, bottom);
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
    if (!tinyui_window_is_valid(window)
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
    if (!tinyui_window_is_valid(window) || gap < 0) {
        return -1;
    }

    return tinyui_window_apply_generic_gap_impl(window, gap);
}


/**
 * @brief Get padding left of window
 *
 * @param[out] window Window instance
 * @return -1 on failure
 */

int tinyui_window_get_padding_left(struct tinyui_window *window)
{
    ldWindow_t *ld_window;

    if (!tinyui_window_is_valid(window)) {
        return -1;
    }

    ld_window = tinyui_window_get_ld_window(window);
    if (ld_window == 0 || ld_window->pLayoutPaddingGroup == 0) {
        return -1;
    }
    return ld_window->pLayoutPaddingGroup->left;
}

/**
 * @brief Get padding top of window
 *
 * @param[out] window Window instance
 * @return -1 on failure
 */

int tinyui_window_get_padding_top(struct tinyui_window *window)
{
    ldWindow_t *ld_window;

    if (!tinyui_window_is_valid(window)) {
        return -1;
    }

    ld_window = tinyui_window_get_ld_window(window);
    if (ld_window == 0 || ld_window->pLayoutPaddingGroup == 0) {
        return -1;
    }
    return ld_window->pLayoutPaddingGroup->top;
}

/**
 * @brief Get padding right of window
 *
 * @param[out] window Window instance
 * @return -1 on failure
 */

int tinyui_window_get_padding_right(struct tinyui_window *window)
{
    ldWindow_t *ld_window;

    if (!tinyui_window_is_valid(window)) {
        return -1;
    }

    ld_window = tinyui_window_get_ld_window(window);
    if (ld_window == 0 || ld_window->pLayoutPaddingGroup == 0) {
        return -1;
    }
    return ld_window->pLayoutPaddingGroup->right;
}

/**
 * @brief Get padding bottom of window
 *
 * @param[out] window Window instance
 * @return -1 on failure
 */

int tinyui_window_get_padding_bottom(struct tinyui_window *window)
{
    ldWindow_t *ld_window;

    if (!tinyui_window_is_valid(window)) {
        return -1;
    }

    ld_window = tinyui_window_get_ld_window(window);
    if (ld_window == 0 || ld_window->pLayoutPaddingGroup == 0) {
        return -1;
    }
    return ld_window->pLayoutPaddingGroup->bottom;
}
