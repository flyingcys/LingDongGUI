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

#include "tinyui.h"

static void make_ui(struct tinyui_window *win)
{
    struct tinyui_label *title;
    struct tinyui_calendar *calendar;
    static const int cols[] = {320, 0};
    static const int rows[] = {28, 196, 0};

    tinyui_grid_set_columns(win, cols, 2);
    tinyui_grid_set_rows(win, rows, 3);
    tinyui_grid_set_gap(win, 12, 12);
    tinyui_grid_set_align(win, TINYUI_ALIGN_START, TINYUI_ALIGN_START);
    tinyui_window_set_padding_group(win, 24, 24, 24, 24);

    title = tinyui_label_create(win, "title");
    if (title != 0) {
        tinyui_label_set_text(title, "Calendar");
        tinyui_widget_set_size((struct tinyui_widget *)title, 220, 28);
        tinyui_widget_set_grid_cell((struct tinyui_widget *)title,
                                    0, 0, 1, 1,
                                    TINYUI_ALIGN_START,
                                    TINYUI_ALIGN_CENTER);
    }

    calendar = tinyui_calendar_create_with_props(
        win,
        &(struct tinyui_calendar_props){
            .id = "calendar",
            .year = 2026,
            .month = 6,
            .day = 15,
            .width = 280,
            .height = 180,
            .show_header = 1,
            .header_format = "yyyy/mm/dd",
        });
    if (calendar != 0) {
        tinyui_widget_set_grid_cell((struct tinyui_widget *)calendar,
                                    0, 1, 1, 1,
                                    TINYUI_ALIGN_START,
                                    TINYUI_ALIGN_CENTER);
    }
}

/**
 * @brief Application entry point
 *
 * @return 0 on success, -1 on failure
 */

int main(void)
{
    struct tinyui_app *app;
    struct tinyui_window *win;

    app = tinyui_app_create();
    if (app == 0) {
        return 1;
    }

    win = tinyui_window_create(app, "root");
    if (win == 0) {
        tinyui_app_destroy(app);
        return 1;
    }

    make_ui(win);
    return tinyui_app_run(app, win);
}
