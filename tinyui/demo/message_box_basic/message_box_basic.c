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

#include "message_box_basic/message_box_basic.h"
#include "tinyui.h"

#include <stdio.h>

static void on_confirm(tinyui_obj_t *box, void *user_data)
{
    (void)box;
    (void)user_data;
    printf("TINYUI_EVENT_TRACE_LINE=message_box:CONFIRM\n");
    fflush(stdout);
}

tinyui_result_t tinyui_demo_message_box_basic_build(tinyui_obj_t *screen)
{
    tinyui_obj_t *title;
    tinyui_obj_t *message_box;
    static const char *buttons[] = {"OK", NULL};

    if (screen == NULL) {
        return TINYUI_ERROR_INVALID_ARG;
    }

    (void)tinyui_obj_set_bg_color(screen, 0xF6F8FAU);

    title = tinyui_label_create(screen);
    message_box = tinyui_message_box_create(screen);
    if (title == NULL || message_box == NULL) {
        return TINYUI_ERROR_NO_MEMORY;
    }

    if (tinyui_obj_set_pos(title, 32, 24) != TINYUI_OK
        || tinyui_obj_set_size(title, 220, 28) != TINYUI_OK
        || tinyui_label_set_text(title, "Message Box") != 0
        || tinyui_label_set_text_color(title, 0x102030U) != 0) {
        return TINYUI_ERROR_BACKEND;
    }

    if (tinyui_obj_set_pos(message_box, 90, 80) != TINYUI_OK
        || tinyui_message_box_set_layout(message_box, 300, 160) != 0
        || tinyui_message_box_set_title(message_box, "Update") != 0
        || tinyui_message_box_set_message(message_box, "Apply settings?") != 0
        || tinyui_message_box_set_buttons(message_box, buttons, 1) != 0
        || tinyui_message_box_set_bg_color(message_box, 0xE8EEF5U) != 0) {
        return TINYUI_ERROR_BACKEND;
    }
    tinyui_message_box_set_on_confirm(message_box, on_confirm, NULL);

    printf("TINYUI_SCENARIO=message_box_basic\n");
    fflush(stdout);
    return TINYUI_OK;
}
