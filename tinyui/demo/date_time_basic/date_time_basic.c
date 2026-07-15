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

#include "date_time_basic/date_time_basic.h"
#include "tinyui.h"

#include <stdio.h>

static tinyui_font_t s_font;

tinyui_result_t tinyui_demo_date_time_basic_build(tinyui_obj_t *screen)
{
    tinyui_obj_t *title;
    tinyui_obj_t *date_time;

    if (screen == NULL) {
        return TINYUI_ERROR_INVALID_ARG;
    }

    (void)tinyui_obj_set_bg_color(screen, 0xF6F8FAU);

    if (tinyui_font_from_builtin(TINYUI_FONT_6X8, &s_font) != TINYUI_OK) {
        /* Font optional; continue with default. */
    }

    title = tinyui_label_create(screen);
    date_time = tinyui_date_time_create(screen);
    if (title == NULL || date_time == NULL) {
        return TINYUI_ERROR_NO_MEMORY;
    }

    if (tinyui_obj_set_pos(title, 32, 32) != TINYUI_OK
        || tinyui_obj_set_size(title, 200, 28) != TINYUI_OK
        || tinyui_label_set_text(title, "Date Time") != 0
        || tinyui_label_set_text_color(title, 0x102030U) != 0) {
        return TINYUI_ERROR_BACKEND;
    }

    if (tinyui_obj_set_pos(date_time, 32, 80) != TINYUI_OK
        || tinyui_obj_set_size(date_time, 280, 32) != TINYUI_OK
        || tinyui_date_time_set_format(date_time, "yyyy-mm-dd hh:nn:ss") != 0
        || tinyui_date_time_set_date(date_time, 2026, 5, 31) != 0
        || tinyui_date_time_set_time(date_time, 12, 34, 56) != 0
        || tinyui_date_time_set_use_system_time(date_time, 0) != 0
        || tinyui_date_time_set_text_color(date_time, 0x102030U) != 0
        || tinyui_date_time_set_bg_color(date_time, 0xE8EEF5U) != 0) {
        return TINYUI_ERROR_BACKEND;
    }
    (void)tinyui_date_time_set_font(date_time, &s_font);

    printf("TINYUI_SCENARIO=date_time_basic\n");
    fflush(stdout);
    return TINYUI_OK;
}
