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
    struct tinyui_combo_box *combo_box;
    static const int cols[] = {240, 0};
    static const int rows[] = {28, 36, 0};

    tinyui_grid_set_columns(win, cols, 2);
    tinyui_grid_set_rows(win, rows, 3);
    tinyui_grid_set_gap(win, 12, 12);
    tinyui_grid_set_align(win, TINYUI_ALIGN_START, TINYUI_ALIGN_START);
    tinyui_window_set_padding_group(win, 24, 24, 24, 24);

    title = tinyui_label_create(win, "title");
    combo_box = tinyui_combo_box_create_with_props(
        win,
        &(struct tinyui_combo_box_props){
            .id = "combo_box",
            .width = 220,
            .height = 32,
        });

    if (title != 0) {
        tinyui_label_set_text(title, "Combo Box");
        tinyui_widget_set_grid_cell((struct tinyui_widget *)title,
                                    0, 0, 1, 1,
                                    TINYUI_ALIGN_START,
                                    TINYUI_ALIGN_CENTER);
    }

    if (combo_box != 0) {
        tinyui_combo_box_add_item(combo_box, "wifi", "Wi-Fi");
        tinyui_combo_box_add_item(combo_box, "bluetooth", "Bluetooth");
        tinyui_combo_box_add_item(combo_box, "display", "Display");
        tinyui_combo_box_set_selected_index(combo_box, 1);
        tinyui_widget_set_grid_cell((struct tinyui_widget *)combo_box,
                                    0, 1, 1, 1,
                                    TINYUI_ALIGN_START,
                                    TINYUI_ALIGN_CENTER);
    }
}

/**
 * @brief Application entry point
 *
 * @return 0 on success
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
    if (tinyui_app_run(app, win) != 0) {
        tinyui_app_destroy(app);
        return 1;
    }

    tinyui_app_destroy(app);
    return 0;
}
