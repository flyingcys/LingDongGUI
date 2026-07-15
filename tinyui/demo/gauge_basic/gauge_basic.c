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

#include "gauge_basic/gauge_basic.h"
#include "tinyui.h"

#include <stdio.h>

static tinyui_image_source_t s_bg;
static tinyui_image_source_t s_pointer;

static void on_gauge_event(const tinyui_event_t *event)
{
    if (event == NULL || event->code != TINYUI_EVENT_CLICKED) {
        return;
    }
    printf("TINYUI_EVENT_TRACE_LINE=gauge:CLICKED\n");
    fflush(stdout);
}

tinyui_result_t tinyui_demo_gauge_basic_build(tinyui_obj_t *screen)
{
    tinyui_obj_t *title;
    tinyui_obj_t *gauge;
    tinyui_result_t ev_rc;

    if (screen == NULL) {
        return TINYUI_ERROR_INVALID_ARG;
    }

    (void)tinyui_obj_set_bg_color(screen, 0xF6F8FAU);

    (void)tinyui_image_source_from_builtin(TINYUI_BUILTIN_IMAGE_GAUGE_BG, &s_bg);
    (void)tinyui_image_source_from_builtin(TINYUI_BUILTIN_IMAGE_GAUGE_POINTER, &s_pointer);

    title = tinyui_label_create(screen);
    gauge = tinyui_gauge_create(screen);
    if (title == NULL || gauge == NULL) {
        return TINYUI_ERROR_NO_MEMORY;
    }

    if (tinyui_obj_set_pos(title, 32, 24) != TINYUI_OK
        || tinyui_obj_set_size(title, 200, 28) != TINYUI_OK
        || tinyui_label_set_text(title, "Gauge") != 0
        || tinyui_label_set_text_color(title, 0x102030U) != 0) {
        return TINYUI_ERROR_BACKEND;
    }

    if (tinyui_obj_set_pos(gauge, 160, 80) != TINYUI_OK
        || tinyui_obj_set_size(gauge, 160, 160) != TINYUI_OK
        || tinyui_gauge_set_angle(gauge, 72.0f) != 0
        || tinyui_gauge_set_pointer_color(gauge, 0xD97706U) != 0
        || tinyui_gauge_set_auto_move(gauge, 0) != 0) {
        return TINYUI_ERROR_BACKEND;
    }
    (void)tinyui_gauge_set_bg_source(gauge, &s_bg);
    (void)tinyui_gauge_set_pointer_source(gauge, &s_pointer);

    ev_rc = tinyui_obj_add_event_cb(gauge,
                                    TINYUI_EVENT_MASK(TINYUI_EVENT_CLICKED),
                                    on_gauge_event,
                                    NULL,
                                    NULL);
    if (ev_rc == TINYUI_ERROR_NOT_SUPPORTED) {
        /* keep visible demo even if click is unsupported */
    } else if (ev_rc != TINYUI_OK) {
        return ev_rc;
    }

    printf("TINYUI_SCENARIO=gauge_basic\n");
    fflush(stdout);
    return TINYUI_OK;
}
