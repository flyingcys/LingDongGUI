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
#include "picoui/layout.h"

static int picoui_window_is_valid(struct picoui_window *window)
{
    return window != 0;
}

/**
 * @brief Set flow of flex widget
 *
 * @param[in] window Window instance
 * @param[in] flow flow
 * @return -1 on failure
 */

int picoui_flex_set_flow(struct picoui_window *window, enum picoui_flex_flow flow)
{
    if (!picoui_window_is_valid(window)) {
        return -1;
    }

    window->flex_flow = flow;
    return picoui_backend_window_set_flex_flow(window, flow);
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

int picoui_flex_set_align(struct picoui_window *window,
                          enum picoui_align main_align,
                          enum picoui_align cross_align,
                          enum picoui_align track_align)
{
    if (!picoui_window_is_valid(window)) {
        return -1;
    }

    window->flex_main_align = main_align;
    window->flex_cross_align = cross_align;
    window->flex_track_align = track_align;
    return picoui_backend_window_set_flex_align(window, main_align, cross_align, track_align);
}

/**
 * @brief Set gap of flex widget
 *
 * @param[in] window Window instance
 * @param[in] item_gap item gap
 * @param[in] track_gap track gap
 * @return -1 on failure
 */

int picoui_flex_set_gap(struct picoui_window *window, int item_gap, int track_gap)
{
    if (!picoui_window_is_valid(window) || item_gap < 0 || track_gap < 0) {
        return -1;
    }

    window->flex_item_gap = item_gap;
    window->flex_track_gap = track_gap;
    return picoui_backend_window_set_flex_gap(window, item_gap, track_gap);
}
