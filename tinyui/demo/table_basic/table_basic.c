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

#include "table_basic/table_basic.h"
#include "tinyui.h"

static void make_ui(struct tinyui_window *win)
{
    struct tinyui_label *title;
    struct tinyui_label *hint;
    struct tinyui_table *table;
    static const int cols[] = {260, 0};
    static const int rows[] = {28, 22, 120, 0};

    tinyui_grid_set_columns(win, cols, 2);
    tinyui_grid_set_rows(win, rows, 4);
    tinyui_grid_set_gap(win, 12, 12);
    tinyui_grid_set_align(win, TINYUI_ALIGN_START, TINYUI_ALIGN_START);
    tinyui_window_set_padding(win, 24, 24, 24, 24);

    title = tinyui_label_create(win, "title");
    hint = tinyui_label_create(win, "hint");
    table = tinyui_table_create_with_props(
        win,
        &(struct tinyui_table_props){
            .id = "table",
            .rows = 3,
            .columns = 3,
            .width = 220,
            .height = 120,
        });

    if (title != 0) {
        tinyui_label_set_text(title, "Table");
        tinyui_widget_set_size((struct tinyui_widget *)title, 220, 28);
        tinyui_widget_set_grid_cell((struct tinyui_widget *)title,
                                    0, 0, 1, 1,
                                    TINYUI_ALIGN_START,
                                    TINYUI_ALIGN_CENTER);
    }

    if (hint != 0) {
        tinyui_label_set_text(hint, "Editable cell reuses the shared R1 commit boundary.");
        tinyui_widget_set_size((struct tinyui_widget *)hint, 260, 22);
        tinyui_widget_set_grid_cell((struct tinyui_widget *)hint,
                                    0, 1, 1, 1,
                                    TINYUI_ALIGN_START,
                                    TINYUI_ALIGN_CENTER);
    }

    if (table != 0) {
        tinyui_table_set_cell_text(table, 0, 0, "A1");
        tinyui_table_set_cell_text(table, 0, 1, "B1");
        tinyui_table_set_cell_text(table, 0, 2, "C1");
        tinyui_table_set_cell_text(table, 1, 0, "A2");
        tinyui_table_set_cell_text(table, 1, 1, "Edit");
        tinyui_table_set_cell_text(table, 1, 2, "C2");
        tinyui_table_set_cell_text(table, 2, 0, "A3");
        tinyui_table_set_cell_text(table, 2, 1, "B3");
        tinyui_table_set_cell_text(table, 2, 2, "C3");
        tinyui_table_set_cell_editable(table, 1, 1, 1, 16);
        tinyui_table_set_current_cell(table, 1, 1);
        tinyui_widget_set_bg_color((struct tinyui_widget *)table, 0xD8EBD0U);
        tinyui_widget_set_text_color((struct tinyui_widget *)table, 0x203020U);
        tinyui_widget_set_border_color((struct tinyui_widget *)table, 0x418F1FU);
        tinyui_widget_set_radius((struct tinyui_widget *)table, 4);
        tinyui_widget_set_padding((struct tinyui_widget *)table, 6);
        tinyui_widget_set_grid_cell((struct tinyui_widget *)table,
                                    0, 2, 1, 1,
                                    TINYUI_ALIGN_START,
                                    TINYUI_ALIGN_CENTER);
    }
}

void tinyui_demo_table_basic(void)
{
    tinyui_obj_t *screen = tinyui_screen_create();
    struct tinyui_window *win = (struct tinyui_window *)screen;
    if (win == 0) return;
    make_ui(win);
    tinyui_screen_load(screen);
}
