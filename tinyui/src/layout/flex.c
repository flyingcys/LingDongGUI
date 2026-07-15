/*
 * Copyright (c) 2023-2026 flyingcys (flyingcys@gmail.com). All rights reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "internal/window_internal.h"
#include "layout/layout.h"

#include <limits.h>

static struct tinyui_window *s_window_of(tinyui_obj_t *container)
{
    struct tinyui_widget *widget = (struct tinyui_widget *)container;

    if (widget == 0
        || (widget->kind != TINYUI_BACKEND_WIDGET_WINDOW
            && widget->kind != TINYUI_BACKEND_WIDGET_BACKGROUND)) {
        return 0;
    }
    return (struct tinyui_window *)widget;
}

static int s_flex_flow_to_ld(tinyui_flex_flow_t flow, ldFlexFlow_t *out)
{
    if (out == 0) {
        return -1;
    }
    switch (flow) {
    case TINYUI_FLEX_FLOW_ROW:
        *out = ldFlexFlowRow;
        return 0;
    case TINYUI_FLEX_FLOW_COLUMN:
        *out = ldFlexFlowColumn;
        return 0;
    case TINYUI_FLEX_FLOW_ROW_WRAP:
        *out = ldFlexFlowRowWrap;
        return 0;
    case TINYUI_FLEX_FLOW_COLUMN_WRAP:
        *out = ldFlexFlowColumnWrap;
        return 0;
    case TINYUI_FLEX_FLOW_ROW_REVERSE:
        *out = ldFlexFlowRowReverse;
        return 0;
    case TINYUI_FLEX_FLOW_COLUMN_REVERSE:
        *out = ldFlexFlowColumnReverse;
        return 0;
    case TINYUI_FLEX_FLOW_ROW_WRAP_REVERSE:
        *out = ldFlexFlowRowWrapReverse;
        return 0;
    case TINYUI_FLEX_FLOW_COLUMN_WRAP_REVERSE:
        *out = ldFlexFlowColumnWrapReverse;
        return 0;
    default:
        return -1;
    }
}

static int s_flex_main_align_to_ld(tinyui_align_t align, ldFlexMainAlign_t *out)
{
    if (out == 0) {
        return -1;
    }
    switch (align) {
    case TINYUI_ALIGN_START:
    case TINYUI_ALIGN_STRETCH:
        *out = ldFlexMainAlignStart;
        return 0;
    case TINYUI_ALIGN_CENTER:
        *out = ldFlexMainAlignCenter;
        return 0;
    case TINYUI_ALIGN_END:
        *out = ldFlexMainAlignEnd;
        return 0;
    case TINYUI_ALIGN_SPACE_EVENLY:
        *out = ldFlexMainAlignSpaceEvenly;
        return 0;
    case TINYUI_ALIGN_SPACE_AROUND:
        *out = ldFlexMainAlignSpaceAround;
        return 0;
    case TINYUI_ALIGN_SPACE_BETWEEN:
        *out = ldFlexMainAlignSpaceBetween;
        return 0;
    default:
        return -1;
    }
}

static int s_flex_cross_align_to_ld(tinyui_align_t align, ldFlexCrossAlign_t *out)
{
    if (out == 0) {
        return -1;
    }
    switch (align) {
    case TINYUI_ALIGN_CENTER:
        *out = ldFlexCrossAlignCenter;
        return 0;
    case TINYUI_ALIGN_END:
        *out = ldFlexCrossAlignEnd;
        return 0;
    case TINYUI_ALIGN_START:
    case TINYUI_ALIGN_STRETCH:
    case TINYUI_ALIGN_SPACE_EVENLY:
    case TINYUI_ALIGN_SPACE_AROUND:
    case TINYUI_ALIGN_SPACE_BETWEEN:
        *out = ldFlexCrossAlignStart;
        return 0;
    default:
        return -1;
    }
}

static int s_flex_track_align_to_ld(tinyui_align_t align, ldFlexTrackAlign_t *out)
{
    if (out == 0) {
        return -1;
    }
    switch (align) {
    case TINYUI_ALIGN_CENTER:
        *out = ldFlexTrackAlignCenter;
        return 0;
    case TINYUI_ALIGN_END:
        *out = ldFlexTrackAlignEnd;
        return 0;
    case TINYUI_ALIGN_SPACE_BETWEEN:
        *out = ldFlexTrackAlignSpaceBetween;
        return 0;
    case TINYUI_ALIGN_SPACE_AROUND:
        *out = ldFlexTrackAlignSpaceAround;
        return 0;
    case TINYUI_ALIGN_SPACE_EVENLY:
        *out = ldFlexTrackAlignSpaceEvenly;
        return 0;
    case TINYUI_ALIGN_START:
    case TINYUI_ALIGN_STRETCH:
        *out = ldFlexTrackAlignStart;
        return 0;
    default:
        return -1;
    }
}

tinyui_result_t tinyui_flex_set_flow(tinyui_obj_t *container,
                                     tinyui_flex_flow_t flow)
{
    struct tinyui_window *window = s_window_of(container);
    ldWindow_t *ld_window;
    ldFlexFlow_t native_flow;

    if (window == 0) {
        return TINYUI_ERROR_INVALID_OBJECT;
    }
    if (s_flex_flow_to_ld(flow, &native_flow) != 0) {
        return TINYUI_ERROR_INVALID_ARG;
    }
    ld_window = tinyui_window_ld_of(window);
    if (ld_window == 0) {
        return TINYUI_ERROR_INVALID_OBJECT;
    }
    window->flex_flow = (enum tinyui_flex_flow)flow;
    ldWindowSetFlexFlow(ld_window, native_flow);
    tinyui_window_sync_padding(window);
    return TINYUI_OK;
}

tinyui_result_t tinyui_flex_set_align(tinyui_obj_t *container,
                                      tinyui_align_t main_align,
                                      tinyui_align_t cross_align,
                                      tinyui_align_t track_align)
{
    struct tinyui_window *window = s_window_of(container);
    ldWindow_t *ld_window;
    ldFlexMainAlign_t native_main;
    ldFlexCrossAlign_t native_cross;
    ldFlexTrackAlign_t native_track;

    if (window == 0) {
        return TINYUI_ERROR_INVALID_OBJECT;
    }
    if (s_flex_main_align_to_ld(main_align, &native_main) != 0
        || s_flex_cross_align_to_ld(cross_align, &native_cross) != 0
        || s_flex_track_align_to_ld(track_align, &native_track) != 0) {
        return TINYUI_ERROR_INVALID_ARG;
    }
    ld_window = tinyui_window_ld_of(window);
    if (ld_window == 0) {
        return TINYUI_ERROR_INVALID_OBJECT;
    }
    window->flex_main_align = (enum tinyui_align)main_align;
    window->flex_cross_align = (enum tinyui_align)cross_align;
    window->flex_track_align = (enum tinyui_align)track_align;
    ldWindowSetFlexAlign(ld_window, native_main, native_cross);
    ldWindowSetFlexTrackAlign(ld_window, native_track);
    tinyui_window_sync_padding(window);
    return TINYUI_OK;
}

tinyui_result_t tinyui_flex_set_gap(tinyui_obj_t *container,
                                    int item_gap,
                                    int track_gap)
{
    struct tinyui_window *window = s_window_of(container);
    ldWindow_t *ld_window;

    if (window == 0) {
        return TINYUI_ERROR_INVALID_OBJECT;
    }
    if (item_gap < 0 || track_gap < 0 || item_gap > INT16_MAX || track_gap > INT16_MAX) {
        return TINYUI_ERROR_OUT_OF_RANGE;
    }
    ld_window = tinyui_window_ld_of(window);
    if (ld_window == 0) {
        return TINYUI_ERROR_INVALID_OBJECT;
    }
    window->flex_item_gap = item_gap;
    window->flex_track_gap = track_gap;
    ldWindowSetFlexGap(ld_window, (int16_t)item_gap, (int16_t)track_gap);
    tinyui_window_sync_padding(window);
    return TINYUI_OK;
}
