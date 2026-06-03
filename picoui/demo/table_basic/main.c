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

#include "picoui/picoui.h"

static void make_ui(struct picoui_window *win)
{
    struct picoui_label *title;
    struct picoui_label *hint;
    struct picoui_table *table;
    static const int cols[] = {260, 0};
    static const int rows[] = {28, 22, 120, 0};

    picoui_grid_set_columns(win, cols, 2);
    picoui_grid_set_rows(win, rows, 4);
    picoui_grid_set_gap(win, 12, 12);
    picoui_grid_set_align(win, PICOUI_ALIGN_START, PICOUI_ALIGN_START);
    picoui_window_set_padding_group(win, 24, 24, 24, 24);

    title = picoui_label_create(win, "title");
    hint = picoui_label_create(win, "hint");
    table = picoui_table_create_with_props(
        win,
        &(struct picoui_table_props){
            .id = "table",
            .rows = 3,
            .columns = 3,
            .width = 220,
            .height = 120,
        });

    if (title != 0) {
        picoui_label_set_text(title, "Table");
        picoui_widget_set_size((struct picoui_widget *)title, 220, 28);
        picoui_widget_set_grid_cell((struct picoui_widget *)title,
                                    0, 0, 1, 1,
                                    PICOUI_ALIGN_START,
                                    PICOUI_ALIGN_CENTER);
    }

    if (hint != 0) {
        picoui_label_set_text(hint, "Editable cell reuses the shared R1 commit boundary.");
        picoui_widget_set_size((struct picoui_widget *)hint, 260, 22);
        picoui_widget_set_grid_cell((struct picoui_widget *)hint,
                                    0, 1, 1, 1,
                                    PICOUI_ALIGN_START,
                                    PICOUI_ALIGN_CENTER);
    }

    if (table != 0) {
        picoui_table_set_cell_text(table, 0, 0, "A1");
        picoui_table_set_cell_text(table, 0, 1, "B1");
        picoui_table_set_cell_text(table, 0, 2, "C1");
        picoui_table_set_cell_text(table, 1, 0, "A2");
        picoui_table_set_cell_text(table, 1, 1, "Edit");
        picoui_table_set_cell_text(table, 1, 2, "C2");
        picoui_table_set_cell_text(table, 2, 0, "A3");
        picoui_table_set_cell_text(table, 2, 1, "B3");
        picoui_table_set_cell_text(table, 2, 2, "C3");
        picoui_table_set_cell_editable(table, 1, 1, 1, 16);
        picoui_table_set_current_cell(table, 1, 1);
        picoui_widget_set_bg_color((struct picoui_widget *)table, 0xD8EBD0U);
        picoui_widget_set_text_color((struct picoui_widget *)table, 0x203020U);
        picoui_widget_set_border_color((struct picoui_widget *)table, 0x418F1FU);
        picoui_widget_set_radius((struct picoui_widget *)table, 4);
        picoui_widget_set_padding((struct picoui_widget *)table, 6);
        picoui_widget_set_grid_cell((struct picoui_widget *)table,
                                    0, 2, 1, 1,
                                    PICOUI_ALIGN_START,
                                    PICOUI_ALIGN_CENTER);
    }
}

/**
 * @brief Application entry point
 *
 * @return 0 on success, -1 on failure
 */

int main(void)
{
    struct picoui_app *app;
    struct picoui_window *win;

    app = picoui_app_create();
    if (app == 0) {
        return 1;
    }

    win = picoui_window_create(app, "root");
    if (win == 0) {
        picoui_app_destroy(app);
        return 1;
    }

    make_ui(win);

    return picoui_app_run(app, win);
}
