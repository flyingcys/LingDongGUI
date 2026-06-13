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
 * Unified TinyUI demo runner.
 *
 * This is the canonical startup path for all TinyUI demos.
 * Demos build their UI on the screen between screen_create and screen_load.
 *
 * Canonical startup sequence:
 *   tinyui_init()
 *   tinyui_screen_create()
 *   demo build API on screen
 *   tinyui_screen_load()
 *   while (1) { tinyui_timer_handler(); }
 *   tinyui_deinit()
 */

#include "basic_widgets/basic_widgets.h"
#include "settings_panel/settings_panel.h"
#include "runtime.h"
#include "widget.h"
#include <stdio.h>

int main(void)
{
    tinyui_obj_t *screen;

    if (tinyui_init() != 0) {
        return 1;
    }

    screen = tinyui_screen_create();
    if (screen == 0) {
        tinyui_deinit();
        return 1;
    }

    /* Call demo build API to create UI on screen.
     * To switch demo, change the TINYUI_DEMO_SETTINGS_PANEL compile
     * definition, or edit the conditional below directly. */
#if defined(TINYUI_DEMO_SETTINGS_PANEL)
    if (tinyui_demo_settings_panel_build(screen) != 0) {
#else
    if (tinyui_demo_basic_widgets_build(screen) != 0) {
#endif
        tinyui_deinit();
        return 1;
    }

    if (tinyui_screen_load(screen) != 0) {
        tinyui_deinit();
        return 1;
    }

    printf("PICOUI_RUNTIME_LOOP\n");
    fflush(stdout);

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
