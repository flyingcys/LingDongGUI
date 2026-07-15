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

#include "clock_basic/clock_basic.h"
#include "tinyui.h"

#include <stdio.h>

static tinyui_obj_t *s_clock;

static void on_clock_timer(tinyui_timer_t *timer, void *user_data)
{
    tinyui_obj_t *clock = (tinyui_obj_t *)user_data;

    (void)timer;
    if (clock == NULL) {
        return;
    }
    /* Keep system-time path armed; callback only uses canonical setters. */
    (void)tinyui_clock_set_use_system_time(clock, 1);
    (void)tinyui_clock_set_step_second(clock, 1);
}

tinyui_result_t tinyui_demo_clock_basic_build(tinyui_obj_t *screen)
{
    tinyui_obj_t *title;
    tinyui_obj_t *clock;
    tinyui_timer_t *timer;

    if (screen == NULL) {
        return TINYUI_ERROR_INVALID_ARG;
    }

    (void)tinyui_obj_set_bg_color(screen, 0xF6F8FAU);

    title = tinyui_label_create(screen);
    clock = tinyui_clock_create(screen);
    if (title == NULL || clock == NULL) {
        return TINYUI_ERROR_NO_MEMORY;
    }

    if (tinyui_obj_set_pos(title, 32, 16) != TINYUI_OK
        || tinyui_obj_set_size(title, 200, 28) != TINYUI_OK
        || tinyui_label_set_text(title, "Clock") != 0
        || tinyui_label_set_text_color(title, 0x102030U) != 0) {
        return TINYUI_ERROR_BACKEND;
    }

    if (tinyui_obj_set_pos(clock, 140, 60) != TINYUI_OK
        || tinyui_obj_set_size(clock, 200, 200) != TINYUI_OK
        || tinyui_clock_set_use_system_time(clock, 1) != 0
        || tinyui_clock_set_step_second(clock, 1) != 0) {
        return TINYUI_ERROR_BACKEND;
    }

    s_clock = clock;
    timer = tinyui_timer_create(1000U, true, on_clock_timer, clock);
    if (timer == NULL) {
        return TINYUI_ERROR_NO_MEMORY;
    }
    if (tinyui_timer_start(timer) != TINYUI_OK) {
        return TINYUI_ERROR_BACKEND;
    }

    printf("TINYUI_SCENARIO=clock_basic\n");
    fflush(stdout);
    return TINYUI_OK;
}
