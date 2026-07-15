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

#ifndef TINYUI_CLOCK_H
#define TINYUI_CLOCK_H

#include "tinyui_config.h"
#if !TINYUI_ENABLE_CLOCK
#  ifndef TINYUI_INTERNAL_FEATURE_HEADER
#  error "TINYUI_ENABLE_CLOCK is disabled"
#  endif
#endif

#include "core/obj.h"
#include <stdint.h>

struct tinyui_image_source;

typedef enum tinyui_clock_field {
    TINYUI_CLOCK_FIELD_ID = UINT32_C(1) << 0,
    TINYUI_CLOCK_FIELD_STYLE_CLASS = UINT32_C(1) << 1,
    TINYUI_CLOCK_FIELD_USER_DATA = UINT32_C(1) << 2,
    TINYUI_CLOCK_FIELD_BACKGROUND_SOURCE = UINT32_C(1) << 3,
    TINYUI_CLOCK_FIELD_HOUR_POINTER_SOURCE = UINT32_C(1) << 4,
    TINYUI_CLOCK_FIELD_MINUTE_POINTER_SOURCE = UINT32_C(1) << 5,
    TINYUI_CLOCK_FIELD_SECOND_POINTER_SOURCE = UINT32_C(1) << 6,
    TINYUI_CLOCK_FIELD_MASK_COLOR = UINT32_C(1) << 7,
    TINYUI_CLOCK_FIELD_HOUR_ANCHOR_X = UINT32_C(1) << 8,
    TINYUI_CLOCK_FIELD_HOUR_ANCHOR_Y = UINT32_C(1) << 9,
    TINYUI_CLOCK_FIELD_MINUTE_ANCHOR_X = UINT32_C(1) << 10,
    TINYUI_CLOCK_FIELD_MINUTE_ANCHOR_Y = UINT32_C(1) << 11,
    TINYUI_CLOCK_FIELD_SECOND_ANCHOR_X = UINT32_C(1) << 12,
    TINYUI_CLOCK_FIELD_SECOND_ANCHOR_Y = UINT32_C(1) << 13,
    TINYUI_CLOCK_FIELD_STEP_SECOND = UINT32_C(1) << 14,
} tinyui_clock_field_t;

typedef struct tinyui_clock_props {
    uint32_t fields;
    uint16_t id;
    const char *style_class;
    void *user_data;
    struct tinyui_image_source *background_source;
    struct tinyui_image_source *hour_pointer_source;
    struct tinyui_image_source *minute_pointer_source;
    struct tinyui_image_source *second_pointer_source;
    unsigned int mask_color;
    float hour_anchor_x;
    float hour_anchor_y;
    float minute_anchor_x;
    float minute_anchor_y;
    float second_anchor_x;
    float second_anchor_y;
    int step_second;
} tinyui_clock_props_t;

tinyui_obj_t *tinyui_clock_create(tinyui_obj_t *parent);

tinyui_obj_t *tinyui_clock_create_with_props(tinyui_obj_t *parent,
                                               const tinyui_clock_props_t *props);

int tinyui_clock_set_use_system_time(tinyui_obj_t *clock, int enabled);

int tinyui_clock_set_auto_sys_time(tinyui_obj_t *clock, int enabled);

int tinyui_clock_get_use_system_time(const tinyui_obj_t *clock);

int tinyui_clock_set_step_second(tinyui_obj_t *clock, int step_second);

int tinyui_clock_get_step_second(const tinyui_obj_t *clock);

int tinyui_clock_set_background_image(tinyui_obj_t *clock, struct tinyui_image_source *source);

int tinyui_clock_set_background_source(tinyui_obj_t *clock, struct tinyui_image_source *source);

int tinyui_clock_set_hour_pointer_image(tinyui_obj_t *clock, struct tinyui_image_source *source);

int tinyui_clock_set_hour_pointer_source(tinyui_obj_t *clock, struct tinyui_image_source *source);

int tinyui_clock_set_minute_pointer_image(tinyui_obj_t *clock, struct tinyui_image_source *source);

int tinyui_clock_set_minute_pointer_source(tinyui_obj_t *clock, struct tinyui_image_source *source);

int tinyui_clock_set_second_pointer_image(tinyui_obj_t *clock, struct tinyui_image_source *source);

int tinyui_clock_set_second_pointer_source(tinyui_obj_t *clock, struct tinyui_image_source *source);

int tinyui_clock_set_mask_color(tinyui_obj_t *clock, unsigned int mask_color);

int tinyui_clock_set_hour_anchor(tinyui_obj_t *clock, float x, float y);

int tinyui_clock_set_minute_anchor(tinyui_obj_t *clock, float x, float y);

int tinyui_clock_set_second_anchor(tinyui_obj_t *clock, float x, float y);

#endif /* TINYUI_CLOCK_H */
