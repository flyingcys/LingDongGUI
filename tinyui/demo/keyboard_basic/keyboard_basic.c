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

#include "keyboard_basic/keyboard_basic.h"
#include "tinyui.h"

#include <string.h>

tinyui_result_t tinyui_demo_keyboard_basic_build(tinyui_obj_t *screen)
{
    tinyui_obj_t *line_edit;
    tinyui_obj_t *keyboard;
    tinyui_line_edit_props_t line_edit_props;
    tinyui_keyboard_props_t keyboard_props;

    if (screen == NULL) {
        return TINYUI_ERROR_INVALID_ARG;
    }

    memset(&line_edit_props, 0, sizeof(line_edit_props));
    line_edit_props.fields = TINYUI_LINE_EDIT_FIELD_TEXT
        | TINYUI_LINE_EDIT_FIELD_KEYBOARD_BINDING
        | TINYUI_LINE_EDIT_FIELD_WIDTH | TINYUI_LINE_EDIT_FIELD_HEIGHT;
    line_edit_props.text = "abc";
    line_edit_props.keyboard_binding = 1U;
    line_edit_props.width = 220;
    line_edit_props.height = 32;

    memset(&keyboard_props, 0, sizeof(keyboard_props));
    keyboard_props.fields = TINYUI_KEYBOARD_FIELD_WIDTH | TINYUI_KEYBOARD_FIELD_HEIGHT;
    keyboard_props.width = 320;
    keyboard_props.height = 160;

    line_edit = tinyui_line_edit_create_with_props(screen, &line_edit_props);
    keyboard = tinyui_keyboard_create_with_props(screen, &keyboard_props);
    if (line_edit == NULL || keyboard == NULL) {
        return TINYUI_ERROR_NO_MEMORY;
    }

    if (tinyui_line_edit_set_keyboard_widget(line_edit, keyboard) != 0
        || tinyui_keyboard_update(keyboard) != 0) {
        return TINYUI_ERROR_BACKEND;
    }

    return TINYUI_OK;
}
