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

#include "layout_grid/layout_grid.h"
#include "tinyui.h"

tinyui_result_t tinyui_demo_layout_grid_build(tinyui_obj_t *screen)
{
    static const tinyui_grid_track_t cols[] = {
        {TINYUI_GRID_UNIT_PX, 80},
        {TINYUI_GRID_UNIT_FR, 1},
    };
    static const tinyui_grid_track_t rows[] = {
        {TINYUI_GRID_UNIT_PX, 32},
        {TINYUI_GRID_UNIT_FR, 1},
    };
    tinyui_obj_t *title;
    tinyui_obj_t *left;
    tinyui_obj_t *right;
    tinyui_result_t result;

    if (screen == NULL) {
        return TINYUI_ERROR_INVALID_ARG;
    }

    if (tinyui_window_set_layout_type(screen, TINYUI_WINDOW_LAYOUT_GRID) != 0) {
        return TINYUI_ERROR_BACKEND;
    }

    result = tinyui_grid_set_columns(screen, cols, 2);
    if (result != TINYUI_OK) {
        return result;
    }
    result = tinyui_grid_set_rows(screen, rows, 2);
    if (result != TINYUI_OK) {
        return result;
    }
    result = tinyui_grid_set_gap(screen, 8, 8);
    if (result != TINYUI_OK) {
        return result;
    }

    title = tinyui_label_create(screen);
    left = tinyui_button_create(screen);
    right = tinyui_button_create(screen);
    if (title == NULL || left == NULL || right == NULL) {
        return TINYUI_ERROR_NO_MEMORY;
    }

    if (tinyui_label_set_text(title, "Grid") != 0
        || tinyui_button_set_text(left, "A") != 0
        || tinyui_button_set_text(right, "B") != 0) {
        return TINYUI_ERROR_BACKEND;
    }

    if (tinyui_obj_set_grid_cell(title,
                                 0, 0, 2, 1,
                                 TINYUI_ALIGN_START,
                                 TINYUI_ALIGN_CENTER) != TINYUI_OK
        || tinyui_obj_set_grid_cell(left,
                                    0, 1, 1, 1,
                                    TINYUI_ALIGN_STRETCH,
                                    TINYUI_ALIGN_STRETCH) != TINYUI_OK
        || tinyui_obj_set_grid_cell(right,
                                    1, 1, 1, 1,
                                    TINYUI_ALIGN_STRETCH,
                                    TINYUI_ALIGN_STRETCH) != TINYUI_OK) {
        return TINYUI_ERROR_BACKEND;
    }

    return TINYUI_OK;
}
