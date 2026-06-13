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
#include "layout.h"

static struct tinyui_backend_widget *tinyui_window_get_backend(struct tinyui_window *window)
{
    if (window == 0 || window->widget.backend_widget == 0) {
        return 0;
    }

    return (struct tinyui_backend_widget *)window->widget.backend_widget;
}

static int tinyui_window_is_valid(struct tinyui_window *window)
{
    return tinyui_window_get_backend(window) != 0;
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
    if (!tinyui_window_is_valid(window)) {
        return -1;
    }

    return tinyui_window_apply_flex_flow(window, flow);
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
    if (!tinyui_window_is_valid(window)) {
        return -1;
    }

    return tinyui_window_apply_flex_align(window, main_align, cross_align, track_align);
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
    if (!tinyui_window_is_valid(window) || item_gap < 0 || track_gap < 0) {
        return -1;
    }

    return tinyui_window_apply_flex_gap(window, item_gap, track_gap);
}
