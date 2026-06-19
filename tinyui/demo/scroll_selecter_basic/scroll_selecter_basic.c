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

#include "scroll_selecter_basic/scroll_selecter_basic.h"
#include "tinyui.h"

static void make_ui(struct tinyui_window *win)
{
    struct tinyui_label *title;
    struct tinyui_label *hint;
    struct tinyui_scroll_selecter *scroll_selecter;
    static const int cols[] = {260, 0};
    static const int rows[] = {28, 22, 120, 0};

    tinyui_grid_set_columns(win, cols, 2);
    tinyui_grid_set_rows(win, rows, 4);
    tinyui_grid_set_gap(win, 12, 12);
    tinyui_grid_set_align(win, TINYUI_ALIGN_START, TINYUI_ALIGN_START);
    tinyui_window_set_padding(win, 24, 24, 24, 24);

    title = tinyui_label_create(win, "title");
    hint = tinyui_label_create(win, "hint");
    scroll_selecter = tinyui_scroll_selecter_create_with_props(
        win,
        &(struct tinyui_scroll_selecter_props){
            .id = "scroll_selecter",
            .width = 220,
            .height = 120,
        });

    if (title != 0) {
        tinyui_label_set_text(title, "Scroll Selecter");
        tinyui_widget_set_grid_cell((struct tinyui_widget *)title,
                                    0, 0, 1, 1,
                                    TINYUI_ALIGN_START,
                                    TINYUI_ALIGN_CENTER);
    }

    if (hint != 0) {
        tinyui_label_set_text(hint, "Swipe to switch between Wi-Fi, Bluetooth and Display.");
        tinyui_widget_set_grid_cell((struct tinyui_widget *)hint,
                                    0, 1, 1, 1,
                                    TINYUI_ALIGN_START,
                                    TINYUI_ALIGN_CENTER);
    }

    if (scroll_selecter != 0) {
        tinyui_scroll_selecter_add_item(scroll_selecter, "wifi", "Wi-Fi");
        tinyui_scroll_selecter_add_item(scroll_selecter, "bluetooth", "Bluetooth");
        tinyui_scroll_selecter_add_item(scroll_selecter, "display", "Display");
        tinyui_scroll_selecter_add_item(scroll_selecter, "sound", "Sound");
        tinyui_scroll_selecter_add_item(scroll_selecter, "privacy", "Privacy");
        tinyui_scroll_selecter_set_selected_index(scroll_selecter, 1);
        tinyui_widget_set_grid_cell((struct tinyui_widget *)scroll_selecter,
                                    0, 2, 1, 1,
                                    TINYUI_ALIGN_START,
                                    TINYUI_ALIGN_CENTER);
    }
}

void tinyui_demo_scroll_selecter_basic(void)
{
    tinyui_obj_t *screen = tinyui_screen_create();
    struct tinyui_window *win = (struct tinyui_window *)screen;
    if (win == 0) return;
    make_ui(win);
    tinyui_screen_load(screen);
}
