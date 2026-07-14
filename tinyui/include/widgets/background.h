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

#ifndef TINYUI_BACKGROUND_H
#define TINYUI_BACKGROUND_H

#include "core/obj.h"
#include <stdint.h>

struct tinyui_image_source;

typedef enum tinyui_background_field {
    TINYUI_BACKGROUND_FIELD_ID = UINT32_C(1) << 0,
    TINYUI_BACKGROUND_FIELD_SOURCE = UINT32_C(1) << 1,
    TINYUI_BACKGROUND_FIELD_COLOR = UINT32_C(1) << 2,
    TINYUI_BACKGROUND_FIELD_OFFSET_X = UINT32_C(1) << 3,
    TINYUI_BACKGROUND_FIELD_OFFSET_Y = UINT32_C(1) << 4,
} tinyui_background_field_t;

struct tinyui_background_props {
    uint32_t fields;
    uint16_t id;
    struct tinyui_image_source *source;
    unsigned int color;
    int offset_x;
    int offset_y;
};
#ifndef TINYUI_BACKGROUND_PROPS_T_DEFINED
typedef struct tinyui_background_props tinyui_background_props_t;
#define TINYUI_BACKGROUND_PROPS_T_DEFINED
#endif


/* creators: tinyui_background_create / create_with_props live in core/runtime.h */

int tinyui_background_set_source(tinyui_obj_t *background, struct tinyui_image_source *source);

int tinyui_background_set_color(tinyui_obj_t *background, unsigned int rgb);

int tinyui_background_get_color(tinyui_obj_t *background, unsigned int *rgb);

int tinyui_background_set_offset(tinyui_obj_t *background, int offset_x, int offset_y);

int tinyui_background_get_offset(tinyui_obj_t *background, int *offset_x, int *offset_y);

#endif /* TINYUI_BACKGROUND_H */
