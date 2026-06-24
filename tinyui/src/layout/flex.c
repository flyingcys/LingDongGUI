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

#include "internal/window_internal.h"
#include "layout.h"

/* ── flex align mappings (flex_main / flex_cross / flex_track) ───────────────
 * These mappings are flex-private and intentionally NOT merged into the core
 * tinyui_align_to_arm2d helper. Keep them here next to the flex setters. */
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

/**
 * @brief Set flow of flex widget
 *
 * @param[in] window Window instance
 * @param[in] flow flow
 * @return -1 on failure
 */
int tinyui_flex_set_flow(struct tinyui_window *window, enum tinyui_flex_flow flow)
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

/**
 * @brief Set align of flex widget
 *
 * @param[in] window Window instance
 * @param[in] main_align main align
 * @param[in] cross_align cross align
 * @param[in] track_align track align
 * @return -1 on failure
 */
int tinyui_flex_set_align(struct tinyui_window *window,
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

/**
 * @brief Set gap of flex widget
 *
 * @param[in] window Window instance
 * @param[in] item_gap item gap
 * @param[in] track_gap track gap
 * @return -1 on failure
 */
int tinyui_flex_set_gap(struct tinyui_window *window, int item_gap, int track_gap)
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
