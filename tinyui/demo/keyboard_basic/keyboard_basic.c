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

#include "keyboard_basic/keyboard_basic.h"
#include "tinyui.h"

static int make_ui(struct tinyui_window *win)
{
    if (tinyui_line_edit_create_with_props(
            win,
            &(struct tinyui_line_edit_props){
                .id = "keyboard_demo_input",
                .text = "abc",
                .keyboard_binding = 1U,
                .width = 220,
                .height = 32,
            }) == 0) {
        return -1;
    }
    if (tinyui_keyboard_create_with_props(
            win,
            &(struct tinyui_keyboard_props){
                .id = "keyboard_demo_keyboard",
                .width = 320,
                .height = 160,
            }) == 0) {
        return -1;
    }
    return 0;
}

void tinyui_demo_keyboard_basic(void)
{
    tinyui_obj_t *screen = tinyui_screen_create();
    struct tinyui_window *win = (struct tinyui_window *)screen;
    if (win == 0) return;
    make_ui(win);
    tinyui_screen_load(screen);
}
