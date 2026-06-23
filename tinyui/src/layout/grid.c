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

/**
 * @brief Set columns of grid widget
 *
 * @param[in] window Window instance
 * @param[in] tracks tracks
 * @param[in] count Count
 * @return -1 on failure
 */

int tinyui_grid_set_columns(struct tinyui_window *window, const int *tracks, int count)
{
    if (window == 0 || window->widget.ld_widget == 0) {
        return -1;
    }

    return tinyui_window_apply_grid_columns(window, tracks, count);
}

/**
 * @brief Set rows of grid widget
 *
 * @param[in] window Window instance
 * @param[in] tracks tracks
 * @param[in] count Count
 * @return -1 on failure
 */

int tinyui_grid_set_rows(struct tinyui_window *window, const int *tracks, int count)
{
    if (window == 0 || window->widget.ld_widget == 0) {
        return -1;
    }

    return tinyui_window_apply_grid_rows(window, tracks, count);
}

/**
 * @brief Set gap of grid widget
 *
 * @param[in] window Window instance
 * @param[in] row_gap row gap
 * @param[in] col_gap col gap
 * @return -1 on failure
 */

int tinyui_grid_set_gap(struct tinyui_window *window, int row_gap, int col_gap)
{
    if (window == 0 || window->widget.ld_widget == 0 || row_gap < 0 || col_gap < 0) {
        return -1;
    }

    return tinyui_window_apply_grid_gap(window, row_gap, col_gap);
}

/**
 * @brief Set align of grid widget
 *
 * @param[in] window Window instance
 * @param[in] col_align col align
 * @param[in] row_align row align
 * @return -1 on failure
 */

int tinyui_grid_set_align(struct tinyui_window *window,
                          enum tinyui_align col_align,
                          enum tinyui_align row_align)
{
    if (window == 0 || window->widget.ld_widget == 0) {
        return -1;
    }

    return tinyui_window_apply_grid_align(window, col_align, row_align);
}
