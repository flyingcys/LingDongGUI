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

static const int cols[] = {80, -2, 0};
static const int rows[] = {32, -2, 0};

static void make_ui(struct picoui_window *win)
{
    struct picoui_label *title;
    struct picoui_button *left;
    struct picoui_button *right;

    picoui_grid_set_columns(win, cols, 3);
    picoui_grid_set_rows(win, rows, 3);
    picoui_grid_set_gap(win, 8, 8);

    title = picoui_label_create(win, "title");
    left = picoui_button_create(win, "left");
    right = picoui_button_create(win, "right");
    picoui_label_set_text(title, "Grid");
    picoui_button_set_text(left, "A");
    picoui_button_set_text(right, "B");
    picoui_widget_set_grid_cell((struct picoui_widget *)title,
                                0, 0, 2, 1,
                                PICOUI_ALIGN_START,
                                PICOUI_ALIGN_CENTER);
    picoui_widget_set_grid_cell((struct picoui_widget *)left,
                                0, 1, 1, 1,
                                PICOUI_ALIGN_STRETCH,
                                PICOUI_ALIGN_STRETCH);
    picoui_widget_set_grid_cell((struct picoui_widget *)right,
                                1, 1, 1, 1,
                                PICOUI_ALIGN_STRETCH,
                                PICOUI_ALIGN_STRETCH);
}

void tinyui_demo_layout_grid(void)
{
    tinyui_obj_t *screen = tinyui_screen_create();
    struct picoui_window *win = (struct picoui_window *)screen;
    if (win == 0) return;
    make_ui(win);
    tinyui_screen_load(screen);
}
