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

#ifndef TINYUI_LINE_EDIT_H
#define TINYUI_LINE_EDIT_H

#include "tinyui_config.h"
#if !TINYUI_ENABLE_LINE_EDIT
#  ifndef TINYUI_INTERNAL_FEATURE_HEADER
#  error "TINYUI_ENABLE_LINE_EDIT is disabled"
#  endif
#endif

#include "core/obj.h"
#include <stdint.h>
#include "layout/layout.h"

struct tinyui_keyboard;

enum tinyui_line_edit_type {
    TINYUI_LINE_EDIT_TYPE_STRING = 0,
    TINYUI_LINE_EDIT_TYPE_INT,
    TINYUI_LINE_EDIT_TYPE_FLOAT,
};

typedef void (*tinyui_line_edit_finished_cb)(tinyui_obj_t *line_edit, void *user_data);

typedef enum tinyui_line_edit_field {
    TINYUI_LINE_EDIT_FIELD_ID = UINT32_C(1) << 0,
    TINYUI_LINE_EDIT_FIELD_TEXT = UINT32_C(1) << 1,
    TINYUI_LINE_EDIT_FIELD_TYPE = UINT32_C(1) << 2,
    TINYUI_LINE_EDIT_FIELD_KEYBOARD_BINDING = UINT32_C(1) << 3,
    TINYUI_LINE_EDIT_FIELD_STYLE_CLASS = UINT32_C(1) << 4,
    TINYUI_LINE_EDIT_FIELD_USER_DATA = UINT32_C(1) << 5,
    TINYUI_LINE_EDIT_FIELD_WIDTH = UINT32_C(1) << 6,
    TINYUI_LINE_EDIT_FIELD_HEIGHT = UINT32_C(1) << 7,
    TINYUI_LINE_EDIT_FIELD_BG_COLOR = UINT32_C(1) << 8,
    TINYUI_LINE_EDIT_FIELD_TEXT_COLOR = UINT32_C(1) << 9,
    TINYUI_LINE_EDIT_FIELD_BORDER_COLOR = UINT32_C(1) << 10,
    TINYUI_LINE_EDIT_FIELD_RADIUS = UINT32_C(1) << 11,
    TINYUI_LINE_EDIT_FIELD_PADDING = UINT32_C(1) << 12,
} tinyui_line_edit_field_t;

typedef struct tinyui_line_edit_props {
    uint32_t fields;
    uint16_t id;
    const char *text;
    enum tinyui_line_edit_type type;
    unsigned int keyboard_binding;
    const char *style_class;
    void *user_data;
    int width;
    int height;
    unsigned int bg_color;
    unsigned int text_color;
    unsigned int border_color;
    int radius;
    int padding;
} tinyui_line_edit_props_t;

tinyui_obj_t *tinyui_line_edit_create(tinyui_obj_t *parent);

tinyui_obj_t *tinyui_line_edit_create_with_props(tinyui_obj_t *parent,
                                               const tinyui_line_edit_props_t *props);

int tinyui_line_edit_set_text(tinyui_obj_t *line_edit, const char *text);

const char *tinyui_line_edit_get_text(const tinyui_obj_t *line_edit);

int tinyui_line_edit_set_align(tinyui_obj_t *line_edit, enum tinyui_align align);

int tinyui_line_edit_set_color(tinyui_obj_t *line_edit, unsigned int text_color, unsigned int background_color, unsigned int frame_color);

int tinyui_line_edit_set_type(tinyui_obj_t *line_edit, enum tinyui_line_edit_type type);

int tinyui_line_edit_get_type(const tinyui_obj_t *line_edit, enum tinyui_line_edit_type *type);

int tinyui_line_edit_set_keyboard(tinyui_obj_t *line_edit, unsigned int keyboard_binding);

int tinyui_line_edit_set_keyboard_binding(tinyui_obj_t *line_edit, unsigned int keyboard_binding);

int tinyui_line_edit_set_keyboard_widget(tinyui_obj_t *line_edit, tinyui_obj_t *keyboard);

int tinyui_line_edit_get_keyboard_binding(const tinyui_obj_t *line_edit, unsigned int *keyboard_binding);

int tinyui_line_edit_get_editing(const tinyui_obj_t *line_edit, int *editing);

int tinyui_line_edit_set_on_edit_finished(tinyui_obj_t *line_edit, tinyui_line_edit_finished_cb cb, void *user_data);

#endif /* TINYUI_LINE_EDIT_H */
