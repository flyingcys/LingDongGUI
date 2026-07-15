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

#ifndef TINYUI_CHECKBOX_H
#define TINYUI_CHECKBOX_H

#include "tinyui_config.h"
#if !TINYUI_ENABLE_CHECKBOX
#  ifndef TINYUI_INTERNAL_FEATURE_HEADER
#  error "TINYUI_ENABLE_CHECKBOX is disabled"
#  endif
#endif

#include "core/obj.h"
#include "core/event.h"
#include <stdint.h>

#ifndef TINYUI_LEGACY_VALUE_CHANGED_CB_DEFINED
#define TINYUI_LEGACY_VALUE_CHANGED_CB_DEFINED
/* Legacy widget-style callback kept for props/compat; set_on_* uses tinyui_event_cb_t. */
typedef void (*tinyui_value_changed_cb)(tinyui_obj_t *obj, int value, void *user_data);
#endif


struct tinyui_image_source;

typedef enum tinyui_checkbox_field {
    TINYUI_CHECKBOX_FIELD_ID = UINT32_C(1) << 0,
    TINYUI_CHECKBOX_FIELD_TEXT = UINT32_C(1) << 1,
    TINYUI_CHECKBOX_FIELD_CHECKED = UINT32_C(1) << 2,
    TINYUI_CHECKBOX_FIELD_ON_TOGGLED = UINT32_C(1) << 3,
    TINYUI_CHECKBOX_FIELD_USER_DATA = UINT32_C(1) << 4,
    TINYUI_CHECKBOX_FIELD_STYLE_CLASS = UINT32_C(1) << 5,
    TINYUI_CHECKBOX_FIELD_WIDTH = UINT32_C(1) << 6,
    TINYUI_CHECKBOX_FIELD_HEIGHT = UINT32_C(1) << 7,
    TINYUI_CHECKBOX_FIELD_BG_COLOR = UINT32_C(1) << 8,
    TINYUI_CHECKBOX_FIELD_TEXT_COLOR = UINT32_C(1) << 9,
    TINYUI_CHECKBOX_FIELD_BORDER_COLOR = UINT32_C(1) << 10,
    TINYUI_CHECKBOX_FIELD_RADIUS = UINT32_C(1) << 11,
    TINYUI_CHECKBOX_FIELD_PADDING = UINT32_C(1) << 12,
    TINYUI_CHECKBOX_FIELD_CHECK_COLOR = UINT32_C(1) << 13,
    TINYUI_CHECKBOX_FIELD_UNCHECKED_SOURCE = UINT32_C(1) << 14,
    TINYUI_CHECKBOX_FIELD_CHECKED_SOURCE = UINT32_C(1) << 15,
    TINYUI_CHECKBOX_FIELD_RADIO_GROUP = UINT32_C(1) << 16,
    TINYUI_CHECKBOX_FIELD_STRING_LEFT_SPACE = UINT32_C(1) << 17,
} tinyui_checkbox_field_t;

typedef struct tinyui_checkbox_props {
    uint32_t fields;
    uint16_t id;
    const char *text;
    int checked;
    tinyui_value_changed_cb on_toggled;
    void *user_data;
    const char *style_class;
    int width;
    int height;
    unsigned int bg_color;
    unsigned int text_color;
    unsigned int border_color;
    int radius;
    int padding;
    unsigned int check_color;
    struct tinyui_image_source *unchecked_source;
    struct tinyui_image_source *checked_source;
    int radio_group;
    int string_left_space;
} tinyui_checkbox_props_t;

tinyui_obj_t *tinyui_checkbox_create(tinyui_obj_t *parent);

tinyui_obj_t *tinyui_checkbox_create_with_props(tinyui_obj_t *parent,
                                               const tinyui_checkbox_props_t *props);

int tinyui_checkbox_set_checked(tinyui_obj_t *checkbox, int checked);

int tinyui_checkbox_is_checked(tinyui_obj_t *checkbox);

int tinyui_checkbox_set_text(tinyui_obj_t *checkbox, const char *text);

int tinyui_checkbox_set_check_color(tinyui_obj_t *checkbox, unsigned int rgb);

int tinyui_checkbox_set_text_color(tinyui_obj_t *checkbox, unsigned int rgb);

int tinyui_checkbox_set_unchecked_source(tinyui_obj_t *checkbox, struct tinyui_image_source *source);

int tinyui_checkbox_set_checked_source(tinyui_obj_t *checkbox, struct tinyui_image_source *source);

int tinyui_checkbox_set_radio_group(tinyui_obj_t *checkbox, int radio_group);

int tinyui_checkbox_set_string_left_space(tinyui_obj_t *checkbox, int space);

/* Narrow forward to tinyui_obj_add_event_cb (unified pool). Replace semantics. */
int tinyui_checkbox_set_on_toggled(tinyui_obj_t *checkbox, tinyui_event_cb_t cb, void *user_data);

#endif /* TINYUI_CHECKBOX_H */
