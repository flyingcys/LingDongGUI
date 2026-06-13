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

#include "date_time_basic/date_time_basic.h"
#include "tinyui.h"

static void make_ui(struct tinyui_window *win)
{
    struct tinyui_label *title;
    struct tinyui_date_time *date_time;
    static const int cols[] = {320, 0};
    static const int rows[] = {28, 44, 0};

    tinyui_grid_set_columns(win, cols, 2);
    tinyui_grid_set_rows(win, rows, 3);
    tinyui_grid_set_gap(win, 16, 16);
    tinyui_grid_set_align(win, TINYUI_ALIGN_CENTER, TINYUI_ALIGN_START);
    tinyui_window_set_padding_group(win, 24, 24, 24, 24);

    title = tinyui_label_create(win, "title");
    date_time = tinyui_date_time_create((struct tinyui_widget *)win, "date_time");

    tinyui_label_set_text(title, "Date Time");
    tinyui_date_time_set_format(date_time, "yyyy-mm-dd hh:nn:ss");
    tinyui_date_time_set_date(date_time, 2026, 5, 31);
    tinyui_date_time_set_time(date_time, 12, 34, 56);

    tinyui_widget_set_size((struct tinyui_widget *)title, 180, 28);
    tinyui_widget_set_size((struct tinyui_widget *)date_time, 240, 32);

    tinyui_widget_set_grid_cell((struct tinyui_widget *)title,
                                0, 0, 1, 1,
                                TINYUI_ALIGN_CENTER,
                                TINYUI_ALIGN_CENTER);
    tinyui_widget_set_grid_cell((struct tinyui_widget *)date_time,
                                0, 1, 1, 1,
                                TINYUI_ALIGN_CENTER,
                                TINYUI_ALIGN_CENTER);
}

void tinyui_demo_date_time_basic(void)
{
    tinyui_obj_t *screen = tinyui_screen_create();
    struct tinyui_window *win = (struct tinyui_window *)screen;

    if (win == 0) {
        return;
    }

    make_ui(win);
    tinyui_screen_load(screen);
}
