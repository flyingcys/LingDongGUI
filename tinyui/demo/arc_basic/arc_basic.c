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

#include "arc_basic/arc_basic.h"
#include "tinyui.h"

#include <stdio.h>

static void on_arc_event(const tinyui_event_t *event)
{
    if (event == NULL || event->code != TINYUI_EVENT_CLICKED) {
        return;
    }
    printf("TINYUI_EVENT_TRACE_LINE=arc:CLICKED\n");
    fflush(stdout);
}

tinyui_result_t tinyui_demo_arc_basic_build(tinyui_obj_t *screen)
{
    tinyui_obj_t *title;
    tinyui_obj_t *arc;
    tinyui_result_t style_rc;

    if (screen == NULL) {
        return TINYUI_ERROR_INVALID_ARG;
    }

    (void)tinyui_obj_set_bg_color(screen, 0xF6F8FAU);

    title = tinyui_label_create(screen);
    arc = tinyui_arc_create(screen);
    if (title == NULL || arc == NULL) {
        return TINYUI_ERROR_NO_MEMORY;
    }

    if (tinyui_obj_set_pos(title, 32, 24) != TINYUI_OK
        || tinyui_obj_set_size(title, 200, 28) != TINYUI_OK
        || tinyui_label_set_text(title, "Arc") != 0
        || tinyui_label_set_text_color(title, 0x102030U) != 0) {
        return TINYUI_ERROR_BACKEND;
    }

    if (tinyui_obj_set_pos(arc, 160, 80) != TINYUI_OK
        || tinyui_obj_set_size(arc, 160, 160) != TINYUI_OK
        || tinyui_arc_set_background_angle(arc, 30.0f, 300.0f) != 0
        || tinyui_arc_set_foreground_angle(arc, 45.0f) != 0
        || tinyui_arc_set_rotation_angle(arc, 12.0f) != 0
        || tinyui_arc_set_color(arc, 0xCCD5E3U, 0x2B6CB0U) != 0) {
        return TINYUI_ERROR_BACKEND;
    }

    style_rc = tinyui_obj_add_event_cb(arc,
                                       TINYUI_EVENT_MASK(TINYUI_EVENT_CLICKED),
                                       on_arc_event,
                                       NULL,
                                       NULL);
    if (style_rc == TINYUI_ERROR_NOT_SUPPORTED) {
        /* Backend may not surface click on arc; keep demo visible. */
    } else if (style_rc != TINYUI_OK) {
        return style_rc;
    }

    printf("TINYUI_SCENARIO=arc_basic\n");
    fflush(stdout);
    return TINYUI_OK;
}
