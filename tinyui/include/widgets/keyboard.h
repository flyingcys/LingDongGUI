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

#ifndef TINYUI_KEYBOARD_H
#define TINYUI_KEYBOARD_H


#include "core/obj.h"
#include <stdint.h>
#include "core/nav.h"




struct tinyui_keyboard_button;

typedef void (*tinyui_keyboard_event_cb)(tinyui_obj_t *keyboard,
                                         unsigned int key_code,
                                         tinyui_signal_t signal,
                                         void *user_data);

typedef void (*tinyui_keyboard_draw_cb)(tinyui_obj_t *keyboard,
                                        const struct tinyui_keyboard_button *button,
                                        void *user_data);

struct tinyui_keyboard_button {
    int x;
    int y;
    int width;
    int height;
    const char *text;
    unsigned int key_code;
    unsigned int press_color;
    unsigned int release_color;
};

typedef enum tinyui_keyboard_field {
    TINYUI_KEYBOARD_FIELD_ID = UINT32_C(1) << 0,
    TINYUI_KEYBOARD_FIELD_STYLE_CLASS = UINT32_C(1) << 1,
    TINYUI_KEYBOARD_FIELD_USER_DATA = UINT32_C(1) << 2,
    TINYUI_KEYBOARD_FIELD_WIDTH = UINT32_C(1) << 3,
    TINYUI_KEYBOARD_FIELD_HEIGHT = UINT32_C(1) << 4,
    TINYUI_KEYBOARD_FIELD_BG_COLOR = UINT32_C(1) << 5,
    TINYUI_KEYBOARD_FIELD_TEXT_COLOR = UINT32_C(1) << 6,
    TINYUI_KEYBOARD_FIELD_BORDER_COLOR = UINT32_C(1) << 7,
    TINYUI_KEYBOARD_FIELD_RADIUS = UINT32_C(1) << 8,
    TINYUI_KEYBOARD_FIELD_PADDING = UINT32_C(1) << 9,
} tinyui_keyboard_field_t;

typedef struct tinyui_keyboard_props {
    uint32_t fields;
    uint16_t id;
    const char *style_class;
    void *user_data;
    int width;
    int height;
    unsigned int bg_color;
    unsigned int text_color;
    unsigned int border_color;
    int radius;
    int padding;
} tinyui_keyboard_props_t;

tinyui_obj_t *tinyui_keyboard_create(tinyui_obj_t *parent);

tinyui_obj_t *tinyui_keyboard_create_with_props(tinyui_obj_t *parent,
                                               const tinyui_keyboard_props_t *props);

int tinyui_keyboard_input_ascii(tinyui_obj_t *keyboard, unsigned int ascii);

int tinyui_keyboard_navigate(tinyui_obj_t *keyboard, int direction);

int tinyui_keyboard_update(tinyui_obj_t *keyboard);

int tinyui_keyboard_button_update(tinyui_obj_t *keyboard, unsigned int key_code);

int tinyui_keyboard_click(tinyui_obj_t *keyboard);

int tinyui_keyboard_exit(tinyui_obj_t *keyboard);

int tinyui_keyboard_set_buttons(tinyui_obj_t *keyboard, const struct tinyui_keyboard_button *buttons, int count);

int tinyui_keyboard_get_buttons(const tinyui_obj_t *keyboard, const struct tinyui_keyboard_button **buttons, int *count);

int tinyui_keyboard_set_on_key_event(tinyui_obj_t *keyboard, tinyui_keyboard_event_cb cb, void *user_data);

int tinyui_keyboard_get_selected_key_code(const tinyui_obj_t *keyboard);

int tinyui_keyboard_set_layout(tinyui_obj_t *keyboard, const struct tinyui_keyboard_button *buttons, int count);

int tinyui_keyboard_set_event_callback(tinyui_obj_t *keyboard, tinyui_keyboard_event_cb cb, void *user_data);

int tinyui_keyboard_set_draw_callback(tinyui_obj_t *keyboard, tinyui_keyboard_draw_cb cb, void *user_data);

#endif /* TINYUI_KEYBOARD_H */
