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

#ifndef TINYUI_BUTTON_H
#define TINYUI_BUTTON_H

#include "core/obj.h"
#include <stdint.h>

#ifndef TINYUI_LEGACY_EVENT_CB_DEFINED
#define TINYUI_LEGACY_EVENT_CB_DEFINED
typedef void (*tinyui_event_cb)(tinyui_obj_t *obj, void *user_data);
#endif


struct tinyui_font;
struct tinyui_image_source;

enum tinyui_button_action_state {
    TINYUI_BUTTON_ACTION_PRESS = 1,
    TINYUI_BUTTON_ACTION_HOLD_DOWN = 2,
    TINYUI_BUTTON_ACTION_RELEASE = 3,
    TINYUI_BUTTON_ACTION_CLICK = 4,
    TINYUI_BUTTON_ACTION_DOUBLE_CLICK = 5,
    TINYUI_BUTTON_ACTION_REPEAT_COUNT = 6,
    TINYUI_BUTTON_ACTION_HOLD_TIME = 7,
    TINYUI_BUTTON_ACTION_LONG_START = 8,
    TINYUI_BUTTON_ACTION_LONG_SHOOT = 9,
};

typedef enum tinyui_button_field {
    TINYUI_BUTTON_FIELD_ID = UINT32_C(1) << 0,
    TINYUI_BUTTON_FIELD_TEXT = UINT32_C(1) << 1,
    TINYUI_BUTTON_FIELD_FONT = UINT32_C(1) << 2,
    TINYUI_BUTTON_FIELD_RELEASE_IMAGE = UINT32_C(1) << 3,
    TINYUI_BUTTON_FIELD_PRESS_IMAGE = UINT32_C(1) << 4,
    TINYUI_BUTTON_FIELD_TRANSPARENT = UINT32_C(1) << 5,
    TINYUI_BUTTON_FIELD_CHECKABLE = UINT32_C(1) << 6,
    TINYUI_BUTTON_FIELD_KEY_VALUE = UINT32_C(1) << 7,
    TINYUI_BUTTON_FIELD_PRESSED = UINT32_C(1) << 8,
    TINYUI_BUTTON_FIELD_WIDTH = UINT32_C(1) << 9,
    TINYUI_BUTTON_FIELD_HEIGHT = UINT32_C(1) << 10,
    TINYUI_BUTTON_FIELD_ON_CLICKED = UINT32_C(1) << 11,
    TINYUI_BUTTON_FIELD_USER_DATA = UINT32_C(1) << 12,
    TINYUI_BUTTON_FIELD_STYLE_CLASS = UINT32_C(1) << 13,
    TINYUI_BUTTON_FIELD_BG_COLOR = UINT32_C(1) << 14,
    TINYUI_BUTTON_FIELD_TEXT_COLOR = UINT32_C(1) << 15,
    TINYUI_BUTTON_FIELD_BORDER_COLOR = UINT32_C(1) << 16,
    TINYUI_BUTTON_FIELD_RADIUS = UINT32_C(1) << 17,
    TINYUI_BUTTON_FIELD_PADDING = UINT32_C(1) << 18,
} tinyui_button_field_t;

typedef struct tinyui_button_props {
    uint32_t fields;
    uint16_t id;
    const char *text;
    const struct tinyui_font *font;
    struct tinyui_image_source *release_image;
    struct tinyui_image_source *press_image;
    int transparent;
    int checkable;
    unsigned int key_value;
    int pressed;
    int width;
    int height;
    tinyui_event_cb on_clicked;
    void *user_data;
    const char *style_class;
    unsigned int bg_color;
    unsigned int text_color;
    unsigned int border_color;
    int radius;
    int padding;
} tinyui_button_props_t;

tinyui_obj_t *tinyui_button_create(tinyui_obj_t *parent);

tinyui_obj_t *tinyui_button_create_with_props(tinyui_obj_t *parent,
                                               const tinyui_button_props_t *props);

int tinyui_button_set_text(tinyui_obj_t *button, const char *text);

int tinyui_button_get_text(tinyui_obj_t *button, const char **text);

int tinyui_button_set_font(tinyui_obj_t *button, const struct tinyui_font *font);

int tinyui_button_get_font(tinyui_obj_t *button, const struct tinyui_font **font);

int tinyui_button_set_color(tinyui_obj_t *button, unsigned int release_color, unsigned int press_color);

int tinyui_button_get_release_color(tinyui_obj_t *button, unsigned int *rgb);

int tinyui_button_get_press_color(tinyui_obj_t *button, unsigned int *rgb);

int tinyui_button_set_text_color(tinyui_obj_t *button, unsigned int text_color);

int tinyui_button_get_text_color(tinyui_obj_t *button, unsigned int *rgb);

int tinyui_button_set_release_image(tinyui_obj_t *button, struct tinyui_image_source *source);

int tinyui_button_set_press_image(tinyui_obj_t *button, struct tinyui_image_source *source);

int tinyui_button_set_image(tinyui_obj_t *button, struct tinyui_image_source *release_source, struct tinyui_image_source *press_source);

int tinyui_button_set_transparent(tinyui_obj_t *button, int transparent);

int tinyui_button_get_transparent(tinyui_obj_t *button, int *transparent);

int tinyui_button_set_checkable(tinyui_obj_t *button, int checkable);

int tinyui_button_get_checkable(tinyui_obj_t *button, int *checkable);

int tinyui_button_set_key_value(tinyui_obj_t *button, unsigned int key_value);

int tinyui_button_get_key_value(tinyui_obj_t *button, unsigned int *key_value);


int tinyui_button_set_pressed(tinyui_obj_t *button, int pressed);


int tinyui_button_get_pressed(tinyui_obj_t *button, int *pressed);

int tinyui_button_get_pressed_by_name_id(const tinyui_obj_t *root, int name_id, int *pressed);

int tinyui_button_get_action_state_by_name_id(const tinyui_obj_t *root, int name_id, enum tinyui_button_action_state action);

int tinyui_button_set_on_clicked(tinyui_obj_t *button, tinyui_event_cb cb, void *user_data);

int tinyui_button_set_on_pressed(tinyui_obj_t *button, tinyui_event_cb cb, void *user_data);

int tinyui_button_set_on_released(tinyui_obj_t *button, tinyui_event_cb cb, void *user_data);

#endif /* TINYUI_BUTTON_H */
