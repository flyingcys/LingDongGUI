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

static ldFlexFlow_t s_flex_flow_to_ld(tinyui_flex_flow_t flow)
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
        return ldFlexFlowRow;
    default:
        return (ldFlexFlow_t)-1;
    }
}

static ldFlexMainAlign_t s_flex_main_align_to_ld(tinyui_align_t align)
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
    case TINYUI_ALIGN_START:
    case TINYUI_ALIGN_STRETCH:
        return ldFlexMainAlignStart;
    default:
        return (ldFlexMainAlign_t)-1;
    }
}

static ldFlexCrossAlign_t s_flex_cross_align_to_ld(tinyui_align_t align)
{
    switch (align) {
    case TINYUI_ALIGN_CENTER:
        return ldFlexCrossAlignCenter;
    case TINYUI_ALIGN_END:
        return ldFlexCrossAlignEnd;
    case TINYUI_ALIGN_START:
    case TINYUI_ALIGN_STRETCH:
    case TINYUI_ALIGN_SPACE_EVENLY:
    case TINYUI_ALIGN_SPACE_AROUND:
    case TINYUI_ALIGN_SPACE_BETWEEN:
        return ldFlexCrossAlignStart;
    default:
        return (ldFlexCrossAlign_t)-1;
    }
}

static ldFlexTrackAlign_t s_flex_track_align_to_ld(tinyui_align_t align)
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
    case TINYUI_ALIGN_START:
    case TINYUI_ALIGN_STRETCH:
        return ldFlexTrackAlignStart;
    default:
        return (ldFlexTrackAlign_t)-1;
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
    native_flow = s_flex_flow_to_ld(flow);
    if ((int)native_flow < 0) {
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
    native_main = s_flex_main_align_to_ld(main_align);
    native_cross = s_flex_cross_align_to_ld(cross_align);
    native_track = s_flex_track_align_to_ld(track_align);
    if ((int)native_main < 0 || (int)native_cross < 0 || (int)native_track < 0) {
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
