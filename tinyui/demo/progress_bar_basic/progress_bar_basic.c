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

#include "progress_bar_basic/progress_bar_basic.h"
#include "tinyui.h"

#include <stdio.h>

tinyui_result_t tinyui_demo_progress_bar_basic_build(tinyui_obj_t *screen)
{
    tinyui_obj_t *title;
    tinyui_obj_t *primary;
    tinyui_obj_t *secondary;

    if (screen == NULL) {
        return TINYUI_ERROR_INVALID_ARG;
    }

    (void)tinyui_obj_set_bg_color(screen, 0xF6F8FAU);

    title = tinyui_label_create(screen);
    primary = tinyui_progress_bar_create(screen);
    secondary = tinyui_progress_bar_create(screen);
    if (title == NULL || primary == NULL || secondary == NULL) {
        return TINYUI_ERROR_NO_MEMORY;
    }

    if (tinyui_obj_set_pos(title, 32, 24) != TINYUI_OK
        || tinyui_obj_set_size(title, 220, 28) != TINYUI_OK
        || tinyui_label_set_text(title, "Progress Bar") != 0
        || tinyui_label_set_text_color(title, 0x102030U) != 0) {
        return TINYUI_ERROR_BACKEND;
    }

    if (tinyui_obj_set_pos(primary, 32, 80) != TINYUI_OK
        || tinyui_obj_set_size(primary, 320, 24) != TINYUI_OK
        || tinyui_progress_bar_set_percent(primary, 72) != 0
        || tinyui_progress_bar_set_horizontal(primary, 1) != 0
        || tinyui_progress_bar_set_color(primary, 0xD0D7DEU, 0x1F6FEBU) != 0) {
        return TINYUI_ERROR_BACKEND;
    }

    if (tinyui_obj_set_pos(secondary, 32, 130) != TINYUI_OK
        || tinyui_obj_set_size(secondary, 48, 144) != TINYUI_OK
        || tinyui_progress_bar_set_percent(secondary, 40) != 0
        || tinyui_progress_bar_set_horizontal(secondary, 0) != 0
        || tinyui_progress_bar_set_color(secondary, 0xD0D7DEU, 0x2DA44EU) != 0) {
        return TINYUI_ERROR_BACKEND;
    }

    printf("TINYUI_SCENARIO=progress_bar_basic\n");
    fflush(stdout);
    return TINYUI_OK;
}
