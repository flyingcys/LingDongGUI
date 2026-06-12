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

static void make_ui(struct picoui_window *win)
{
    struct picoui_label *title;
    struct picoui_line_edit *line_edit;
    static const int cols[] = {280, 0};
    static const int rows[] = {28, 36, 0};

    picoui_grid_set_columns(win, cols, 2);
    picoui_grid_set_rows(win, rows, 3);
    picoui_grid_set_gap(win, 12, 12);
    picoui_grid_set_align(win, PICOUI_ALIGN_START, PICOUI_ALIGN_START);
    picoui_window_set_padding_group(win, 24, 24, 24, 24);

    title = picoui_label_create(win, "title");
    line_edit = picoui_line_edit_create_with_props(
        win,
        &(struct picoui_line_edit_props){
            .id = "line_edit",
            .text = "192.168.0.10",
            .type = PICOUI_LINE_EDIT_TYPE_STRING,
            .keyboard_binding = 1U,
            .has_type = 1,
            .has_keyboard_binding = 1,
            .width = 240,
            .height = 32,
        });

    if (title != 0) {
        picoui_label_set_text(title, "Line Edit");
        picoui_widget_set_grid_cell((struct picoui_widget *)title,
                                    0, 0, 1, 1,
                                    PICOUI_ALIGN_START,
                                    PICOUI_ALIGN_CENTER);
    }

    if (line_edit != 0) {
        picoui_widget_set_grid_cell((struct picoui_widget *)line_edit,
                                    0, 1, 1, 1,
                                    PICOUI_ALIGN_START,
                                    PICOUI_ALIGN_CENTER);
    }
}

static int run_demo(void)
{
    struct picoui_app *app = picoui_app_create();
    struct picoui_window *win;

    if (app == 0) {
        return 1;
    }

    win = picoui_window_create(app, "root");
    if (win == 0) {
        picoui_app_destroy(app);
        return 1;
    }

    make_ui(win);
    if (picoui_app_run(app, win) != 0) {
        picoui_app_destroy(app);
        return 1;
    }

    picoui_app_destroy(app);
    return 0;
}

/**
 * @brief Application entry point
 *
 * @return 0 on success, -1 on failure
 */

int main(void)
{
    return run_demo();
}
