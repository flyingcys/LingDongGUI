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

#ifndef TINYUI_PROGRESS_BAR_H
#define TINYUI_PROGRESS_BAR_H

#include "core/obj.h"
#include <stdint.h>

struct tinyui_image_source;

typedef enum tinyui_progress_bar_field {
    TINYUI_PROGRESS_BAR_FIELD_ID = UINT32_C(1) << 0,
    TINYUI_PROGRESS_BAR_FIELD_STYLE_CLASS = UINT32_C(1) << 1,
    TINYUI_PROGRESS_BAR_FIELD_USER_DATA = UINT32_C(1) << 2,
    TINYUI_PROGRESS_BAR_FIELD_PERCENT = UINT32_C(1) << 3,
    TINYUI_PROGRESS_BAR_FIELD_HORIZONTAL = UINT32_C(1) << 4,
    TINYUI_PROGRESS_BAR_FIELD_INVERTED = UINT32_C(1) << 5,
} tinyui_progress_bar_field_t;

typedef struct tinyui_progress_bar_props {
    uint32_t fields;
    uint16_t id;
    const char *style_class;
    void *user_data;
    int percent;
    int horizontal;
    int inverted;
} tinyui_progress_bar_props_t;

tinyui_obj_t *tinyui_progress_bar_create(tinyui_obj_t *parent);

tinyui_obj_t *tinyui_progress_bar_create_with_props(tinyui_obj_t *parent,
                                               const tinyui_progress_bar_props_t *props);

int tinyui_progress_bar_set_percent(tinyui_obj_t *bar, int percent);

int tinyui_progress_bar_get_percent(const tinyui_obj_t *bar);

int tinyui_progress_bar_set_horizontal(tinyui_obj_t *bar, int horizontal);

int tinyui_progress_bar_get_horizontal(const tinyui_obj_t *bar);

int tinyui_progress_bar_set_image(tinyui_obj_t *bar, struct tinyui_image_source *bg_source, struct tinyui_image_source *fg_source);

int tinyui_progress_bar_set_bg_source(tinyui_obj_t *bar, struct tinyui_image_source *source);

int tinyui_progress_bar_set_fg_source(tinyui_obj_t *bar, struct tinyui_image_source *source);

int tinyui_progress_bar_set_frame_source(tinyui_obj_t *bar, struct tinyui_image_source *source);

int tinyui_progress_bar_set_color(tinyui_obj_t *bar, unsigned int bg_color, unsigned int fg_color);

int tinyui_progress_bar_set_frame_color(tinyui_obj_t *bar, unsigned int frame_color, int frame_color_size);

int tinyui_progress_bar_set_inverted(tinyui_obj_t *bar, int inverted);

int tinyui_progress_bar_get_inverted(const tinyui_obj_t *bar);

#endif /* TINYUI_PROGRESS_BAR_H */
