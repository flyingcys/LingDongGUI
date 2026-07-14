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

#ifndef TINYUI_PROGRESS_WHEEL_H
#define TINYUI_PROGRESS_WHEEL_H

#include "core/obj.h"
#include <stdint.h>

typedef enum tinyui_progress_wheel_field {
    TINYUI_PROGRESS_WHEEL_FIELD_ID = UINT32_C(1) << 0,
    TINYUI_PROGRESS_WHEEL_FIELD_STYLE_CLASS = UINT32_C(1) << 1,
    TINYUI_PROGRESS_WHEEL_FIELD_USER_DATA = UINT32_C(1) << 2,
    TINYUI_PROGRESS_WHEEL_FIELD_PERCENT = UINT32_C(1) << 3,
    TINYUI_PROGRESS_WHEEL_FIELD_DOT_ENABLED = UINT32_C(1) << 4,
} tinyui_progress_wheel_field_t;

typedef struct tinyui_progress_wheel_props {
    uint32_t fields;
    uint16_t id;
    const char *style_class;
    void *user_data;
    int percent;
    int dot_enabled;
} tinyui_progress_wheel_props_t;

tinyui_obj_t *tinyui_progress_wheel_create(tinyui_obj_t *parent);

tinyui_obj_t *tinyui_progress_wheel_create_with_props(tinyui_obj_t *parent,
                                               const tinyui_progress_wheel_props_t *props);

int tinyui_progress_wheel_set_percent(tinyui_obj_t *wheel, int percent);

int tinyui_progress_wheel_set_progress(tinyui_obj_t *wheel, int percent);

int tinyui_progress_wheel_get_percent(const tinyui_obj_t *wheel);

int tinyui_progress_wheel_set_wheel_color(tinyui_obj_t *wheel, unsigned int rgb);

int tinyui_progress_wheel_set_dot_color(tinyui_obj_t *wheel, unsigned int rgb);

int tinyui_progress_wheel_set_dot_enabled(tinyui_obj_t *wheel, int enabled);

int tinyui_progress_wheel_get_dot_enabled(const tinyui_obj_t *wheel);

#endif /* TINYUI_PROGRESS_WHEEL_H */
