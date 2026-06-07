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
#include "picoui/port/sdl.h"

static struct picoui_window *g_root_window;

static void make_ui(struct picoui_window *win)
{
    struct picoui_label *title;
    struct picoui_label *hint;
    struct picoui_scroll_selecter *scroll_selecter;
    static const int cols[] = {260, 0};
    static const int rows[] = {28, 22, 120, 0};

    picoui_grid_set_columns(win, cols, 2);
    picoui_grid_set_rows(win, rows, 4);
    picoui_grid_set_gap(win, 12, 12);
    picoui_grid_set_align(win, PICOUI_ALIGN_START, PICOUI_ALIGN_START);
    picoui_window_set_padding_group(win, 24, 24, 24, 24);

    title = picoui_label_create(win, "title");
    hint = picoui_label_create(win, "hint");
    scroll_selecter = picoui_scroll_selecter_create_with_props(
        win,
        &(struct picoui_scroll_selecter_props){
            .id = "scroll_selecter",
            .width = 220,
            .height = 120,
        });

    if (title != 0) {
        picoui_label_set_text(title, "Scroll Selecter");
        picoui_widget_set_grid_cell((struct picoui_widget *)title,
                                    0, 0, 1, 1,
                                    PICOUI_ALIGN_START,
                                    PICOUI_ALIGN_CENTER);
    }

    if (hint != 0) {
        picoui_label_set_text(hint, "Swipe to switch between Wi-Fi, Bluetooth and Display.");
        picoui_widget_set_grid_cell((struct picoui_widget *)hint,
                                    0, 1, 1, 1,
                                    PICOUI_ALIGN_START,
                                    PICOUI_ALIGN_CENTER);
    }

    if (scroll_selecter != 0) {
        picoui_scroll_selecter_add_item(scroll_selecter, "wifi", "Wi-Fi");
        picoui_scroll_selecter_add_item(scroll_selecter, "bluetooth", "Bluetooth");
        picoui_scroll_selecter_add_item(scroll_selecter, "display", "Display");
        picoui_scroll_selecter_add_item(scroll_selecter, "sound", "Sound");
        picoui_scroll_selecter_add_item(scroll_selecter, "privacy", "Privacy");
        picoui_scroll_selecter_set_selected_index(scroll_selecter, 1);
        picoui_widget_set_grid_cell((struct picoui_widget *)scroll_selecter,
                                    0, 2, 1, 1,
                                    PICOUI_ALIGN_START,
                                    PICOUI_ALIGN_CENTER);
    }
}

static int create_demo_ui(void)
{
    struct picoui_screen *screen = picoui_screen_active();
    struct picoui_window *win;

    win = picoui_window_create_root(screen, "root");
    if (win == 0) {
        return -1;
    }

    g_root_window = win;
    make_ui(win);
    if (picoui_screen_load(screen) != 0) {
        g_root_window = 0;
        return -1;
    }

    return 0;
}

/**
 * @brief Application entry point
 *
 * @return 0 on success
 */

int main(void)
{
    int init_rc;
    int timer_rc;

    init_rc = picoui_init();
    if (init_rc != 0) {
        return 1;
    }
    if (picoui_sdl_hal_init(320, 480) != 0) {
        picoui_deinit();
        return 1;
    }
    if (create_demo_ui() != 0 || g_root_window == 0) {
        picoui_deinit();
        return 1;
    }

    while (1) {
        timer_rc = picoui_timer_handler();
        if (timer_rc < 0) {
            picoui_deinit();
            return 1;
        }
        if (timer_rc > 0) {
            picoui_deinit();
            return 0;
        }
    }

    return 0;
}
