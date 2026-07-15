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

#ifndef TINYUI_GAUGE_H
#define TINYUI_GAUGE_H

#include "tinyui_config.h"
#if !TINYUI_ENABLE_GAUGE
#  ifndef TINYUI_INTERNAL_FEATURE_HEADER
#  error "TINYUI_ENABLE_GAUGE is disabled"
#  endif
#endif

#include "core/obj.h"
#include <stdint.h>

struct tinyui_image_source;

typedef enum tinyui_gauge_field {
    TINYUI_GAUGE_FIELD_ID = UINT32_C(1) << 0,
    TINYUI_GAUGE_FIELD_STYLE_CLASS = UINT32_C(1) << 1,
    TINYUI_GAUGE_FIELD_USER_DATA = UINT32_C(1) << 2,
    TINYUI_GAUGE_FIELD_ANGLE = UINT32_C(1) << 3,
    TINYUI_GAUGE_FIELD_BG_SOURCE = UINT32_C(1) << 4,
    TINYUI_GAUGE_FIELD_POINTER_SOURCE = UINT32_C(1) << 5,
    TINYUI_GAUGE_FIELD_CENTRE_OFFSET_X = UINT32_C(1) << 6,
    TINYUI_GAUGE_FIELD_CENTRE_OFFSET_Y = UINT32_C(1) << 7,
    TINYUI_GAUGE_FIELD_POINTER_COLOR = UINT32_C(1) << 8,
    TINYUI_GAUGE_FIELD_AUTO_MOVE = UINT32_C(1) << 9,
} tinyui_gauge_field_t;

typedef struct tinyui_gauge_props {
    uint32_t fields;
    uint16_t id;
    const char *style_class;
    void *user_data;
    float angle;
    struct tinyui_image_source *bg_source;
    struct tinyui_image_source *pointer_source;
    int centre_offset_x;
    int centre_offset_y;
    unsigned int pointer_color;
    int auto_move;
} tinyui_gauge_props_t;

tinyui_obj_t *tinyui_gauge_create(tinyui_obj_t *parent);

tinyui_obj_t *tinyui_gauge_create_with_props(tinyui_obj_t *parent,
                                               const tinyui_gauge_props_t *props);

int tinyui_gauge_set_angle(tinyui_obj_t *gauge, float angle);

float tinyui_gauge_get_angle(const tinyui_obj_t *gauge);

int tinyui_gauge_set_bg_source(tinyui_obj_t *gauge, struct tinyui_image_source *source);

int tinyui_gauge_set_background_image(tinyui_obj_t *gauge, struct tinyui_image_source *source);

int tinyui_gauge_set_pointer_source(tinyui_obj_t *gauge, struct tinyui_image_source *source);

int tinyui_gauge_set_pointer_source_with_origin(tinyui_obj_t *gauge, struct tinyui_image_source *source, int origin_x, int origin_y);

int tinyui_gauge_set_pointer_mask_source(tinyui_obj_t *gauge, struct tinyui_image_source *source, int origin_x, int origin_y);

int tinyui_gauge_set_centre_offset(tinyui_obj_t *gauge, int centre_offset_x, int centre_offset_y);

int tinyui_gauge_set_trail(tinyui_obj_t *gauge, struct tinyui_image_source *bg_trail_source, struct tinyui_image_source *pointer_trail_source);

int tinyui_gauge_set_progress_bar(tinyui_obj_t *gauge, struct tinyui_image_source *bg_progress_source, struct tinyui_image_source *pointer_progress_source);

int tinyui_gauge_set_pointer_color(tinyui_obj_t *gauge, unsigned int pointer_color);

unsigned int tinyui_gauge_get_pointer_color(const tinyui_obj_t *gauge);

int tinyui_gauge_set_auto_move(tinyui_obj_t *gauge, int auto_move);

int tinyui_gauge_get_auto_move(const tinyui_obj_t *gauge);

#endif /* TINYUI_GAUGE_H */
