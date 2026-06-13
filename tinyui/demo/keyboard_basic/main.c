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

/**
 * @brief Application entry point
 *
 * @return 0 on success
 */

int main(void)
{
    struct tinyui_app *app;
    struct tinyui_window *window;

    app = tinyui_app_create();
    if (app == 0) {
        return 1;
    }

    window = tinyui_window_create(app, "keyboard_root");
    if (window == 0) {
        tinyui_app_destroy(app);
        return 1;
    }

    if (tinyui_line_edit_create_with_props(
            window,
            &(struct tinyui_line_edit_props){
                .id = "keyboard_demo_input",
                .text = "abc",
                .keyboard_binding = 1U,
                .has_keyboard_binding = 1,
                .width = 220,
                .height = 32,
            }) == 0) {
        tinyui_app_destroy(app);
        return 1;
    }
    if (tinyui_keyboard_create_with_props(
            window,
            &(struct tinyui_keyboard_props){
                .id = "keyboard_demo_keyboard",
                .width = 320,
                .height = 160,
            }) == 0) {
        tinyui_app_destroy(app);
        return 1;
    }

    if (tinyui_app_run(app, window) != 0) {
        tinyui_app_destroy(app);
        return 1;
    }

    tinyui_app_destroy(app);
    return 0;
}
