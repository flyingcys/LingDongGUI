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

#ifndef PICOUI_LAYOUT_H
#define PICOUI_LAYOUT_H

enum picoui_align {
    PICOUI_ALIGN_START,
    PICOUI_ALIGN_CENTER,
    PICOUI_ALIGN_END,
    PICOUI_ALIGN_STRETCH,
    PICOUI_ALIGN_SPACE_EVENLY,
    PICOUI_ALIGN_SPACE_AROUND,
    PICOUI_ALIGN_SPACE_BETWEEN,
};

enum picoui_flex_flow {
    PICOUI_FLEX_FLOW_ROW,
    PICOUI_FLEX_FLOW_COLUMN,
    PICOUI_FLEX_FLOW_ROW_WRAP,
    PICOUI_FLEX_FLOW_COLUMN_WRAP,
    PICOUI_FLEX_FLOW_ROW_REVERSE,
    PICOUI_FLEX_FLOW_COLUMN_REVERSE,
    PICOUI_FLEX_FLOW_ROW_WRAP_REVERSE,
    PICOUI_FLEX_FLOW_COLUMN_WRAP_REVERSE,
};

struct picoui_window;

/**
 * @brief Set flow of flex widget
 *
 * @param[in] window Window instance
 * @param[in] flow flow
 * @return 0 on success, -1 on failure
 */

int picoui_flex_set_flow(struct picoui_window *window, enum picoui_flex_flow flow);

/**
 * @brief Set align of flex widget
 *
 * @param[in] window Window instance
 * @param[in] main_align main align
 * @param[in] cross_align cross align
 * @param[in] track_align track align
 * @return 0 on success, -1 on failure
 */

int picoui_flex_set_align(struct picoui_window *window,
                          enum picoui_align main_align,
                          enum picoui_align cross_align,
                          enum picoui_align track_align);

/**
 * @brief Set gap of flex widget
 *
 * @param[in] window Window instance
 * @param[in] item_gap item gap
 * @param[in] track_gap track gap
 * @return 0 on success, -1 on failure
 */

int picoui_flex_set_gap(struct picoui_window *window, int item_gap, int track_gap);

/**
 * @brief Set columns of grid widget
 *
 * @param[in] window Window instance
 * @param[in] tracks tracks
 * @param[in] count Count
 * @return 0 on success, -1 on failure
 */

int picoui_grid_set_columns(struct picoui_window *window, const int *tracks, int count);

/**
 * @brief Set rows of grid widget
 *
 * @param[in] window Window instance
 * @param[in] tracks tracks
 * @param[in] count Count
 * @return 0 on success, -1 on failure
 */

int picoui_grid_set_rows(struct picoui_window *window, const int *tracks, int count);

/**
 * @brief Set gap of grid widget
 *
 * @param[in] window Window instance
 * @param[in] row_gap row gap
 * @param[in] col_gap col gap
 * @return 0 on success, -1 on failure
 */

int picoui_grid_set_gap(struct picoui_window *window, int row_gap, int col_gap);

/**
 * @brief Set align of grid widget
 *
 * @param[in] window Window instance
 * @param[in] col_align col align
 * @param[in] row_align row align
 * @return 0 on success, -1 on failure
 */

int picoui_grid_set_align(struct picoui_window *window,
                          enum picoui_align col_align,
                          enum picoui_align row_align);

#endif
