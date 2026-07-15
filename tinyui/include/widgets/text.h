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

#ifndef TINYUI_TEXT_H
#define TINYUI_TEXT_H

#include "tinyui_config.h"
#if !TINYUI_ENABLE_TEXT
#  ifndef TINYUI_INTERNAL_FEATURE_HEADER
#  error "TINYUI_ENABLE_TEXT is disabled"
#  endif
#endif

#include "core/obj.h"
#include <stdint.h>

struct tinyui_font;
struct tinyui_image_source;

typedef enum tinyui_text_field {
    TINYUI_TEXT_FIELD_ID = UINT32_C(1) << 0,
    TINYUI_TEXT_FIELD_TEXT = UINT32_C(1) << 1,
    TINYUI_TEXT_FIELD_FONT = UINT32_C(1) << 2,
    TINYUI_TEXT_FIELD_STYLE_CLASS = UINT32_C(1) << 3,
    TINYUI_TEXT_FIELD_USER_DATA = UINT32_C(1) << 4,
    TINYUI_TEXT_FIELD_WIDTH = UINT32_C(1) << 5,
    TINYUI_TEXT_FIELD_HEIGHT = UINT32_C(1) << 6,
    TINYUI_TEXT_FIELD_BG_COLOR = UINT32_C(1) << 7,
    TINYUI_TEXT_FIELD_TEXT_COLOR = UINT32_C(1) << 8,
    TINYUI_TEXT_FIELD_BORDER_COLOR = UINT32_C(1) << 9,
    TINYUI_TEXT_FIELD_RADIUS = UINT32_C(1) << 10,
    TINYUI_TEXT_FIELD_PADDING = UINT32_C(1) << 11,
} tinyui_text_field_t;

typedef struct tinyui_text_props {
    uint32_t fields;
    uint16_t id;
    const char *text;
    const struct tinyui_font *font;
    const char *style_class;
    void *user_data;
    int width;
    int height;
    unsigned int bg_color;
    unsigned int text_color;
    unsigned int border_color;
    int radius;
    int padding;
} tinyui_text_props_t;

tinyui_obj_t *tinyui_text_create(tinyui_obj_t *parent);

tinyui_obj_t *tinyui_text_create_with_props(tinyui_obj_t *parent,
                                               const tinyui_text_props_t *props);

int tinyui_text_set_text(tinyui_obj_t *text, const char *value);

int tinyui_text_set_static_text(tinyui_obj_t *text, const char *value);

int tinyui_text_set_font(tinyui_obj_t *text, const struct tinyui_font *font);

int tinyui_text_set_transparent(tinyui_obj_t *text, int transparent);

int tinyui_text_set_text_color(tinyui_obj_t *text, unsigned int rgb);

int tinyui_text_set_bg_color(tinyui_obj_t *text, unsigned int rgb);

int tinyui_text_set_background_source(tinyui_obj_t *text, struct tinyui_image_source *source);

int tinyui_text_set_consumed_font(tinyui_obj_t *text, const struct tinyui_font *font);

int tinyui_text_set_scroll_enabled(tinyui_obj_t *text, int enabled);

int tinyui_text_scroll_seek(tinyui_obj_t *text, int offset);

int tinyui_text_scroll_move(tinyui_obj_t *text, int move_value);

#endif /* TINYUI_TEXT_H */
