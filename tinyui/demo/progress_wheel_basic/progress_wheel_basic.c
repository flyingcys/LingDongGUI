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

#include "progress_wheel_basic/progress_wheel_basic.h"
#include "tinyui.h"

#include <stdio.h>

tinyui_result_t tinyui_demo_progress_wheel_basic_build(tinyui_obj_t *screen)
{
    tinyui_obj_t *title;
    tinyui_obj_t *wheel;

    if (screen == NULL) {
        return TINYUI_ERROR_INVALID_ARG;
    }

    (void)tinyui_obj_set_bg_color(screen, 0xF6F8FAU);

    title = tinyui_label_create(screen);
    wheel = tinyui_progress_wheel_create(screen);
    if (title == NULL || wheel == NULL) {
        return TINYUI_ERROR_NO_MEMORY;
    }

    if (tinyui_obj_set_pos(title, 32, 24) != TINYUI_OK
        || tinyui_obj_set_size(title, 240, 28) != TINYUI_OK
        || tinyui_label_set_text(title, "Progress Wheel") != 0
        || tinyui_label_set_text_color(title, 0x102030U) != 0) {
        return TINYUI_ERROR_BACKEND;
    }

    if (tinyui_obj_set_pos(wheel, 192, 100) != TINYUI_OK
        || tinyui_obj_set_size(wheel, 96, 96) != TINYUI_OK
        || tinyui_progress_wheel_set_percent(wheel, 72) != 0
        || tinyui_progress_wheel_set_dot_enabled(wheel, 1) != 0
        || tinyui_progress_wheel_set_dot_color(wheel, 0x1F6FEBU) != 0) {
        return TINYUI_ERROR_BACKEND;
    }

    printf("TINYUI_SCENARIO=progress_wheel_basic\n");
    fflush(stdout);
    return TINYUI_OK;
}
