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

#include "line_edit_basic/line_edit_basic.h"
#include "tinyui.h"

#include <string.h>

tinyui_result_t tinyui_demo_line_edit_basic_build(tinyui_obj_t *screen)
{
    tinyui_obj_t *title;
    tinyui_obj_t *line_edit;
    tinyui_line_edit_props_t line_edit_props;
    tinyui_result_t result;

    if (screen == NULL) {
        return TINYUI_ERROR_INVALID_ARG;
    }

    result = tinyui_flex_set_flow(screen, TINYUI_FLEX_FLOW_COLUMN);
    if (result != TINYUI_OK) {
        return result;
    }
    result = tinyui_flex_set_align(screen,
                                   TINYUI_ALIGN_START,
                                   TINYUI_ALIGN_START,
                                   TINYUI_ALIGN_START);
    if (result != TINYUI_OK) {
        return result;
    }
    result = tinyui_flex_set_gap(screen, 12, 12);
    if (result != TINYUI_OK) {
        return result;
    }
    if (tinyui_window_set_padding(screen, 24, 24, 24, 24) != 0) {
        return TINYUI_ERROR_BACKEND;
    }

    memset(&line_edit_props, 0, sizeof(line_edit_props));
    line_edit_props.fields = TINYUI_LINE_EDIT_FIELD_TEXT
        | TINYUI_LINE_EDIT_FIELD_TYPE
        | TINYUI_LINE_EDIT_FIELD_KEYBOARD_BINDING
        | TINYUI_LINE_EDIT_FIELD_WIDTH | TINYUI_LINE_EDIT_FIELD_HEIGHT;
    line_edit_props.text = "192.168.0.10";
    line_edit_props.type = TINYUI_LINE_EDIT_TYPE_STRING;
    line_edit_props.keyboard_binding = 1U;
    line_edit_props.width = 240;
    line_edit_props.height = 32;

    title = tinyui_label_create(screen);
    line_edit = tinyui_line_edit_create_with_props(screen, &line_edit_props);
    if (title == NULL || line_edit == NULL) {
        return TINYUI_ERROR_NO_MEMORY;
    }

    if (tinyui_label_set_text(title, "Line Edit") != 0) {
        return TINYUI_ERROR_BACKEND;
    }

    return TINYUI_OK;
}
