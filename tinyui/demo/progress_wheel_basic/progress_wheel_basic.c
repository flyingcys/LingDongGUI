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

#include "progress_wheel_basic/progress_wheel_basic.h"
#include "tinyui.h"

static void make_ui(struct picoui_window *win)
{
    struct picoui_label *title;
    struct picoui_progress_wheel *wheel;
    static const int cols[] = {320, 0};
    static const int rows[] = {28, 128, 0};

    picoui_grid_set_columns(win, cols, 2);
    picoui_grid_set_rows(win, rows, 3);
    picoui_grid_set_gap(win, 16, 16);
    picoui_grid_set_align(win, PICOUI_ALIGN_CENTER, PICOUI_ALIGN_START);
    picoui_window_set_padding_group(win, 24, 24, 24, 24);

    title = picoui_label_create(win, "title");
    wheel = picoui_progress_wheel_create((struct picoui_widget *)win, "wheel");

    picoui_label_set_text(title, "Progress Wheel");
    picoui_progress_wheel_set_percent(wheel, 72);
    picoui_widget_set_size((struct picoui_widget *)title, 220, 28);
    picoui_widget_set_size((struct picoui_widget *)wheel, 96, 96);

    picoui_widget_set_grid_cell((struct picoui_widget *)title,
                                0, 0, 1, 1,
                                PICOUI_ALIGN_CENTER,
                                PICOUI_ALIGN_CENTER);
    picoui_widget_set_grid_cell((struct picoui_widget *)wheel,
                                0, 1, 1, 1,
                                PICOUI_ALIGN_CENTER,
                                PICOUI_ALIGN_START);
}

void tinyui_demo_progress_wheel_basic(void)
{
    tinyui_obj_t *screen = tinyui_screen_create();
    struct picoui_window *win = (struct picoui_window *)screen;
    if (win == 0) return;
    make_ui(win);
    tinyui_screen_load(screen);
}
