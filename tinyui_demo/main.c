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

/*
 * TinyUI demo entry point — mirrors lv_port_pc_vscode/main/src/main.c.
 *
 * LVGL pattern:
 *   lv_init();
 *   hal_init(320, 480);
 *   lv_demo_widgets();          // or: lv_demos_create(argv+1, argc-1)
 *   while (1) { lv_timer_handler(); }
 *
 * TinyUI equivalent (HAL init is inside tinyui_init):
 *   tinyui_init();
 *   tinyui_demos_create(argv+1, argc-1);
 *   for (;;) { tinyui_timer_handler(); }
 *   tinyui_deinit();
 *
 * Select demo at runtime:
 *   ./tinyui_demo arc_basic
 *   ./tinyui_demo hello_world
 * No argument: runs the first demo (animation_basic).
 */

#include "tinyui_demos.h"
#include "runtime.h"
#include <stdio.h>

int main(int argc, char **argv)
{
    if (tinyui_init() != 0) {
        return 1;
    }

    if (!tinyui_demos_create(argv + 1, argc - 1)) {
        tinyui_demos_show_help();
        tinyui_deinit();
        return 1;
    }

    for (;;) {
        int step = tinyui_timer_handler();
        if (step < 0) {
            tinyui_deinit();
            return 1;
        }
        if (step > 0) {
            tinyui_deinit();
            return 0;
        }
    }
}
