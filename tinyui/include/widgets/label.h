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

#ifndef TINYUI_LABEL_H
#define TINYUI_LABEL_H

#include "tinyui_config.h"
#if !TINYUI_ENABLE_LABEL
#  ifndef TINYUI_INTERNAL_FEATURE_HEADER
#  error "TINYUI_ENABLE_LABEL is disabled"
#  endif
#endif

#include "layout/layout.h"

#include "core/obj.h"
#include <stdint.h>

struct tinyui_font;
struct tinyui_image_source;

typedef enum tinyui_label_field {
    TINYUI_LABEL_FIELD_ID = UINT32_C(1) << 0,
    TINYUI_LABEL_FIELD_TEXT = UINT32_C(1) << 1,
    TINYUI_LABEL_FIELD_FONT = UINT32_C(1) << 2,
    TINYUI_LABEL_FIELD_STYLE_CLASS = UINT32_C(1) << 3,
    TINYUI_LABEL_FIELD_USER_DATA = UINT32_C(1) << 4,
    TINYUI_LABEL_FIELD_WIDTH = UINT32_C(1) << 5,
    TINYUI_LABEL_FIELD_HEIGHT = UINT32_C(1) << 6,
    TINYUI_LABEL_FIELD_BG_COLOR = UINT32_C(1) << 7,
    TINYUI_LABEL_FIELD_TEXT_COLOR = UINT32_C(1) << 8,
    TINYUI_LABEL_FIELD_BORDER_COLOR = UINT32_C(1) << 9,
    TINYUI_LABEL_FIELD_RADIUS = UINT32_C(1) << 10,
    TINYUI_LABEL_FIELD_PADDING = UINT32_C(1) << 11,
    TINYUI_LABEL_FIELD_TRANSPARENT = UINT32_C(1) << 12,
    TINYUI_LABEL_FIELD_ALIGN = UINT32_C(1) << 13,
    TINYUI_LABEL_FIELD_BACKGROUND_SOURCE = UINT32_C(1) << 14,
} tinyui_label_field_t;

typedef struct tinyui_label_props {
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
    int transparent;
    enum tinyui_align align;
    struct tinyui_image_source *background_source;
} tinyui_label_props_t;

tinyui_obj_t *tinyui_label_create(tinyui_obj_t *parent);

tinyui_obj_t *tinyui_label_create_with_props(tinyui_obj_t *parent,
                                               const tinyui_label_props_t *props);

int tinyui_label_set_text(tinyui_obj_t *label, const char *text);

const char *tinyui_label_get_text(tinyui_obj_t *label);

int tinyui_label_set_font(tinyui_obj_t *label, const struct tinyui_font *font);

int tinyui_label_set_text_color(tinyui_obj_t *label, unsigned int rgb);

int tinyui_label_get_text_color(tinyui_obj_t *label, unsigned int *rgb);

int tinyui_label_set_bg_color(tinyui_obj_t *label, unsigned int rgb);

int tinyui_label_get_bg_color(tinyui_obj_t *label, unsigned int *rgb);

int tinyui_label_set_transparent(tinyui_obj_t *label, int transparent);

int tinyui_label_get_transparent(tinyui_obj_t *label, int *transparent);

int tinyui_label_set_align(tinyui_obj_t *label, enum tinyui_align align);

int tinyui_label_get_align(tinyui_obj_t *label, enum tinyui_align *align);

int tinyui_label_set_text_align(tinyui_obj_t *label, enum tinyui_align x_align, enum tinyui_align y_align);

int tinyui_label_set_background_source(tinyui_obj_t *label, struct tinyui_image_source *source);

#endif /* TINYUI_LABEL_H */
