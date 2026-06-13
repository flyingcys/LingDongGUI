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

#include "list_basic/list_basic.h"
#include "tinyui.h"

static void make_ui(struct picoui_window *win)
{
    struct picoui_label *title;
    struct picoui_list *list;
    static const int cols[] = {220, 0};
    static const int rows[] = {32, 104, 0};

    picoui_grid_set_columns(win, cols, 2);
    picoui_grid_set_rows(win, rows, 3);
    picoui_grid_set_gap(win, 8, 8);
    picoui_grid_set_align(win, PICOUI_ALIGN_START, PICOUI_ALIGN_START);

    title = picoui_label_create(win, "title");
    list = picoui_list_create(win, "list");

    if (title != 0) {
        picoui_label_set_text(title, "List");
        picoui_widget_set_grid_cell((struct picoui_widget *)title,
                                    0, 0, 1, 1,
                                    PICOUI_ALIGN_START,
                                    PICOUI_ALIGN_CENTER);
    }

    if (list != 0) {
        picoui_list_add_item(list, "item_wifi", "Wi-Fi");
        picoui_list_add_item(list, "item_bluetooth", "Bluetooth");
        picoui_list_add_item(list, "item_display", "Display");
        picoui_list_set_selected_index(list, 0);
        picoui_widget_set_grid_cell((struct picoui_widget *)list,
                                    0, 1, 1, 1,
                                    PICOUI_ALIGN_STRETCH,
                                    PICOUI_ALIGN_START);
    }
}

void tinyui_demo_list_basic(void)
{
    tinyui_obj_t *screen = tinyui_screen_create();
    struct picoui_window *win = (struct picoui_window *)screen;
    if (win == 0) return;
    make_ui(win);
    tinyui_screen_load(screen);
}
