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

#ifndef TINYUI_ANIMATION_H
#define TINYUI_ANIMATION_H

#include "tinyui_config.h"
#if !TINYUI_ENABLE_ANIMATION
#  ifndef TINYUI_INTERNAL_FEATURE_HEADER
#  error "TINYUI_ENABLE_ANIMATION is disabled"
#  endif
#endif

#include "core/obj.h"
#include <stdint.h>

struct tinyui_image_source;

typedef enum tinyui_animation_field {
    TINYUI_ANIMATION_FIELD_ID = UINT32_C(1) << 0,
    TINYUI_ANIMATION_FIELD_STYLE_CLASS = UINT32_C(1) << 1,
    TINYUI_ANIMATION_FIELD_USER_DATA = UINT32_C(1) << 2,
    TINYUI_ANIMATION_FIELD_WIDTH = UINT32_C(1) << 3,
    TINYUI_ANIMATION_FIELD_HEIGHT = UINT32_C(1) << 4,
    TINYUI_ANIMATION_FIELD_PERIOD_MS = UINT32_C(1) << 5,
    TINYUI_ANIMATION_FIELD_SOURCE = UINT32_C(1) << 6,
} tinyui_animation_field_t;

typedef struct tinyui_animation_props {
    uint32_t fields;
    uint16_t id;
    const char *style_class;
    void *user_data;
    int width;
    int height;
    int period_ms;
    struct tinyui_image_source *source;
} tinyui_animation_props_t;

tinyui_obj_t *tinyui_animation_create(tinyui_obj_t *parent);

tinyui_obj_t *tinyui_animation_create_with_props(tinyui_obj_t *parent,
                                               const tinyui_animation_props_t *props);

int tinyui_animation_set_source(tinyui_obj_t *animation, struct tinyui_image_source *source);

int tinyui_animation_set_period_ms(tinyui_obj_t *animation, int period_ms);

int tinyui_animation_show_frame(tinyui_obj_t *animation, int frame_index);

#endif /* TINYUI_ANIMATION_H */
