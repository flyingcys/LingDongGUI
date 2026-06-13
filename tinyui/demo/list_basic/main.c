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
    struct tinyui_list *list;
    static const int cols[] = {220, 0};
    static const int rows[] = {32, 104, 0};

    tinyui_grid_set_columns(win, cols, 2);
    tinyui_grid_set_rows(win, rows, 3);
    tinyui_grid_set_gap(win, 8, 8);
    tinyui_grid_set_align(win, TINYUI_ALIGN_START, TINYUI_ALIGN_START);

    title = tinyui_label_create(win, "title");
    list = tinyui_list_create(win, "list");

    if (title != 0) {
        tinyui_label_set_text(title, "List");
        tinyui_widget_set_grid_cell((struct tinyui_widget *)title,
                                    0, 0, 1, 1,
                                    TINYUI_ALIGN_START,
                                    TINYUI_ALIGN_CENTER);
    }

    if (list != 0) {
        tinyui_list_add_item(list, "item_wifi", "Wi-Fi");
        tinyui_list_add_item(list, "item_bluetooth", "Bluetooth");
        tinyui_list_add_item(list, "item_display", "Display");
        tinyui_list_set_selected_index(list, 0);
        tinyui_widget_set_grid_cell((struct tinyui_widget *)list,
                                    0, 1, 1, 1,
                                    TINYUI_ALIGN_STRETCH,
                                    TINYUI_ALIGN_START);
    }
}

static int run_demo(void)
{
    struct tinyui_app *app = tinyui_app_create();
    struct tinyui_window *win;

    if (app == 0) {
        return 1;
    }

    win = tinyui_window_create(app, "root");
    if (win == 0) {
        tinyui_app_destroy(app);
        return 1;
    }

    make_ui(win);
    if (tinyui_app_run(app, win) != 0) {
        tinyui_app_destroy(app);
        return 1;
    }
    tinyui_app_destroy(app);
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
