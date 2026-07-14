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

#ifndef TINYUI_IMAGE_H
#define TINYUI_IMAGE_H

#include "core/obj.h"
#include <stdint.h>
#include "resource/font.h"
#include "resource/image_source.h"

struct tinyui_image_source;

typedef enum tinyui_image_field {
    TINYUI_IMAGE_FIELD_ID = UINT32_C(1) << 0,
    TINYUI_IMAGE_FIELD_SOURCE = UINT32_C(1) << 1,
    TINYUI_IMAGE_FIELD_STYLE_CLASS = UINT32_C(1) << 2,
    TINYUI_IMAGE_FIELD_USER_DATA = UINT32_C(1) << 3,
    TINYUI_IMAGE_FIELD_WIDTH = UINT32_C(1) << 4,
    TINYUI_IMAGE_FIELD_HEIGHT = UINT32_C(1) << 5,
    TINYUI_IMAGE_FIELD_BG_COLOR = UINT32_C(1) << 6,
    TINYUI_IMAGE_FIELD_TEXT_COLOR = UINT32_C(1) << 7,
    TINYUI_IMAGE_FIELD_BORDER_COLOR = UINT32_C(1) << 8,
    TINYUI_IMAGE_FIELD_RADIUS = UINT32_C(1) << 9,
    TINYUI_IMAGE_FIELD_PADDING = UINT32_C(1) << 10,
} tinyui_image_field_t;

typedef struct tinyui_image_props {
    uint32_t fields;
    uint16_t id;
    struct tinyui_image_source *source;
    const char *style_class;
    void *user_data;
    int width;
    int height;
    unsigned int bg_color;
    unsigned int text_color;
    unsigned int border_color;
    int radius;
    int padding;
} tinyui_image_props_t;

tinyui_obj_t *tinyui_image_create(tinyui_obj_t *parent);

tinyui_obj_t *tinyui_image_create_with_props(tinyui_obj_t *parent,
                                               const tinyui_image_props_t *props);

int tinyui_image_set_source(tinyui_obj_t *image, struct tinyui_image_source *source);

int tinyui_image_set_mask_color(tinyui_obj_t *image, unsigned int rgb);

#endif /* TINYUI_IMAGE_H */
