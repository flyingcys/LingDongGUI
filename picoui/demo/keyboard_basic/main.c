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

static int create_demo_ui(void)
{
    struct picoui_screen *screen = picoui_screen_active();
    struct picoui_window *window;

    if (screen == 0) {
        return -1;
    }

    window = picoui_window_create_root(screen, "keyboard_root");
    if (window == 0) {
        return -1;
    }

    g_root_window = window;
    picoui_window_set_padding_group(window, 24, 24, 24, 24);
    picoui_window_set_layout_type(window, PICOUI_WINDOW_LAYOUT_GRID);
    picoui_grid_set_columns(window, (const int[]){320, 0}, 2);
    picoui_grid_set_rows(window, (const int[]){32, 160, 0}, 3);
    picoui_grid_set_gap(window, 16, 16);
    picoui_grid_set_align(window, PICOUI_ALIGN_CENTER, PICOUI_ALIGN_CENTER);

    if (picoui_line_edit_create_with_props(
            window,
            &(struct picoui_line_edit_props){
                .id = "keyboard_demo_input",
                .text = "abc",
                .keyboard_binding = 1U,
                .has_keyboard_binding = 1,
                .width = 220,
                .height = 32,
            }) == 0) {
        g_root_window = 0;
        return -1;
    }
    if (picoui_keyboard_create_with_props(
            window,
            &(struct picoui_keyboard_props){
                .id = "keyboard_demo_keyboard",
                .width = 320,
                .height = 160,
            }) == 0) {
        g_root_window = 0;
        return -1;
    }

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
