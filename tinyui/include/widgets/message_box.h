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

#ifndef TINYUI_MESSAGE_BOX_H
#define TINYUI_MESSAGE_BOX_H

#include "tinyui_config.h"
#if !TINYUI_ENABLE_MESSAGE_BOX
#  ifndef TINYUI_INTERNAL_FEATURE_HEADER
#  error "TINYUI_ENABLE_MESSAGE_BOX is disabled"
#  endif
#endif

#include "core/obj.h"
#include <stdint.h>

typedef void (*tinyui_message_box_callback_t)(tinyui_obj_t *box, void *user_data);

typedef void (*tinyui_message_box_indexed_callback_t)(tinyui_obj_t *box,
                                                      int index,
                                                      void *user_data);

typedef enum tinyui_message_box_field {
    TINYUI_MESSAGE_BOX_FIELD_ID = UINT32_C(1) << 0,
    TINYUI_MESSAGE_BOX_FIELD_STYLE_CLASS = UINT32_C(1) << 1,
    TINYUI_MESSAGE_BOX_FIELD_USER_DATA = UINT32_C(1) << 2,
    TINYUI_MESSAGE_BOX_FIELD_TITLE = UINT32_C(1) << 3,
    TINYUI_MESSAGE_BOX_FIELD_MESSAGE = UINT32_C(1) << 4,
    TINYUI_MESSAGE_BOX_FIELD_CONFIRM_TEXT = UINT32_C(1) << 5,
} tinyui_message_box_field_t;

typedef struct tinyui_message_box_props {
    uint32_t fields;
    uint16_t id;
    const char *style_class;
    void *user_data;
    const char *title;
    const char *message;
    const char *confirm_text;
} tinyui_message_box_props_t;

tinyui_obj_t *tinyui_message_box_create(tinyui_obj_t *parent);

tinyui_obj_t *tinyui_message_box_create_with_props(tinyui_obj_t *parent,
                                               const tinyui_message_box_props_t *props);

int tinyui_message_box_set_title(tinyui_obj_t *box, const char *title);

int tinyui_message_box_set_message(tinyui_obj_t *box, const char *message);

int tinyui_message_box_set_msg(tinyui_obj_t *box, const char *message);

int tinyui_message_box_set_confirm_text(tinyui_obj_t *box, const char *text);

int tinyui_message_box_set_layout(tinyui_obj_t *box, int width, int height);

int tinyui_message_box_set_buttons(tinyui_obj_t *box, const char *const *buttons, int count);

int tinyui_message_box_set_btn(tinyui_obj_t *box, const char *const *buttons, int count);

int tinyui_message_box_set_string_colors(tinyui_obj_t *box, unsigned int title_color, unsigned int message_color, unsigned int button_color);

int tinyui_message_box_set_string_color(tinyui_obj_t *box, unsigned int title_color, unsigned int message_color, unsigned int button_color);

int tinyui_message_box_set_button_colors(tinyui_obj_t *box, unsigned int release_color, unsigned int press_color);

int tinyui_message_box_set_button_color(tinyui_obj_t *box, unsigned int release_color, unsigned int press_color);

int tinyui_message_box_set_bg_color(tinyui_obj_t *box, unsigned int bg_color);

int tinyui_message_box_set_background_color(tinyui_obj_t *box, unsigned int bg_color);

void tinyui_message_box_set_on_confirm(tinyui_obj_t *box, tinyui_message_box_callback_t callback, void *user_data);

void tinyui_message_box_set_callback(tinyui_obj_t *box, tinyui_message_box_callback_t callback, void *user_data);

void tinyui_message_box_set_on_confirm_indexed(tinyui_obj_t *box, tinyui_message_box_indexed_callback_t callback, void *user_data);

const char *tinyui_message_box_get_title(const tinyui_obj_t *box);

const char *tinyui_message_box_get_message(const tinyui_obj_t *box);

const char *tinyui_message_box_get_confirm_text(const tinyui_obj_t *box);

#endif /* TINYUI_MESSAGE_BOX_H */
