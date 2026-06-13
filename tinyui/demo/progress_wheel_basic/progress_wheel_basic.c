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

static void make_ui(struct tinyui_window *win)
{
    struct tinyui_label *title;
    struct tinyui_progress_wheel *wheel;
    static const int cols[] = {320, 0};
    static const int rows[] = {28, 128, 0};

    tinyui_grid_set_columns(win, cols, 2);
    tinyui_grid_set_rows(win, rows, 3);
    tinyui_grid_set_gap(win, 16, 16);
    tinyui_grid_set_align(win, TINYUI_ALIGN_CENTER, TINYUI_ALIGN_START);
    tinyui_window_set_padding_group(win, 24, 24, 24, 24);

    title = tinyui_label_create(win, "title");
    wheel = tinyui_progress_wheel_create((struct tinyui_widget *)win, "wheel");

    tinyui_label_set_text(title, "Progress Wheel");
    tinyui_progress_wheel_set_percent(wheel, 72);
    tinyui_widget_set_size((struct tinyui_widget *)title, 220, 28);
    tinyui_widget_set_size((struct tinyui_widget *)wheel, 96, 96);

    tinyui_widget_set_grid_cell((struct tinyui_widget *)title,
                                0, 0, 1, 1,
                                TINYUI_ALIGN_CENTER,
                                TINYUI_ALIGN_CENTER);
    tinyui_widget_set_grid_cell((struct tinyui_widget *)wheel,
                                0, 1, 1, 1,
                                TINYUI_ALIGN_CENTER,
                                TINYUI_ALIGN_START);
}

void tinyui_demo_progress_wheel_basic(void)
{
    tinyui_obj_t *screen = tinyui_screen_create();
    struct tinyui_window *win = (struct tinyui_window *)screen;
    if (win == 0) return;
    make_ui(win);
    tinyui_screen_load(screen);
}
