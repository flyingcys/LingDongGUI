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

#ifndef TINYUI_ARC_H
#define TINYUI_ARC_H

#include "core/obj.h"
#include <stdint.h>

struct tinyui_image_source;

typedef enum tinyui_arc_field {
    TINYUI_ARC_FIELD_ID = UINT32_C(1) << 0,
    TINYUI_ARC_FIELD_STYLE_CLASS = UINT32_C(1) << 1,
    TINYUI_ARC_FIELD_USER_DATA = UINT32_C(1) << 2,
    TINYUI_ARC_FIELD_BG_START_ANGLE = UINT32_C(1) << 3,
    TINYUI_ARC_FIELD_BG_END_ANGLE = UINT32_C(1) << 4,
    TINYUI_ARC_FIELD_FG_END_ANGLE = UINT32_C(1) << 5,
    TINYUI_ARC_FIELD_ROTATION_ANGLE = UINT32_C(1) << 6,
    TINYUI_ARC_FIELD_QUARTER_SOURCE = UINT32_C(1) << 7,
    TINYUI_ARC_FIELD_HAS_PARENT_COLOR = UINT32_C(1) << 8,
    TINYUI_ARC_FIELD_PARENT_COLOR = UINT32_C(1) << 9,
    TINYUI_ARC_FIELD_BG_COLOR = UINT32_C(1) << 10,
    TINYUI_ARC_FIELD_FG_COLOR = UINT32_C(1) << 11,
} tinyui_arc_field_t;

typedef struct tinyui_arc_props {
    uint32_t fields;
    uint16_t id;
    const char *style_class;
    void *user_data;
    float bg_start_angle;
    float bg_end_angle;
    float fg_end_angle;
    float rotation_angle;
    struct tinyui_image_source *quarter_source;
    int has_parent_color;
    unsigned int parent_color;
    unsigned int bg_color;
    unsigned int fg_color;
} tinyui_arc_props_t;

tinyui_obj_t *tinyui_arc_create(tinyui_obj_t *parent);

tinyui_obj_t *tinyui_arc_create_with_props(tinyui_obj_t *parent,
                                               const tinyui_arc_props_t *props);

int tinyui_arc_set_background_angle(tinyui_obj_t *arc, float bg_start_angle, float bg_end_angle);

int tinyui_arc_set_foreground_angle(tinyui_obj_t *arc, float fg_end_angle);

int tinyui_arc_set_rotation_angle(tinyui_obj_t *arc, float rotation_angle);

int tinyui_arc_set_quarter_source(tinyui_obj_t *arc, struct tinyui_image_source *source);

int tinyui_arc_set_quarter_image(tinyui_obj_t *arc, struct tinyui_image_source *source);

int tinyui_arc_set_parent_color(tinyui_obj_t *arc, unsigned int parent_color);

int tinyui_arc_set_color(tinyui_obj_t *arc, unsigned int bg_color, unsigned int fg_color);

float tinyui_arc_get_background_start_angle(const tinyui_obj_t *arc);

float tinyui_arc_get_background_angle(const tinyui_obj_t *arc);

float tinyui_arc_get_foreground_angle(const tinyui_obj_t *arc);

float tinyui_arc_get_rotation_angle(const tinyui_obj_t *arc);

unsigned int tinyui_arc_get_background_color(const tinyui_obj_t *arc);

unsigned int tinyui_arc_get_foreground_color(const tinyui_obj_t *arc);

#endif /* TINYUI_ARC_H */
