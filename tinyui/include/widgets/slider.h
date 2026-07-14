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

#ifndef TINYUI_SLIDER_H
#define TINYUI_SLIDER_H

#include "core/obj.h"
#include <stdint.h>

#ifndef TINYUI_LEGACY_VALUE_CHANGED_CB_DEFINED
#define TINYUI_LEGACY_VALUE_CHANGED_CB_DEFINED
typedef void (*tinyui_value_changed_cb)(tinyui_obj_t *obj, int value, void *user_data);
#endif


struct tinyui_image_source;

typedef enum tinyui_slider_field {
    TINYUI_SLIDER_FIELD_ID = UINT32_C(1) << 0,
    TINYUI_SLIDER_FIELD_MIN_VALUE = UINT32_C(1) << 1,
    TINYUI_SLIDER_FIELD_MAX_VALUE = UINT32_C(1) << 2,
    TINYUI_SLIDER_FIELD_VALUE = UINT32_C(1) << 3,
    TINYUI_SLIDER_FIELD_ON_VALUE_CHANGED = UINT32_C(1) << 4,
    TINYUI_SLIDER_FIELD_USER_DATA = UINT32_C(1) << 5,
    TINYUI_SLIDER_FIELD_STYLE_CLASS = UINT32_C(1) << 6,
    TINYUI_SLIDER_FIELD_WIDTH = UINT32_C(1) << 7,
    TINYUI_SLIDER_FIELD_HEIGHT = UINT32_C(1) << 8,
    TINYUI_SLIDER_FIELD_BG_COLOR = UINT32_C(1) << 9,
    TINYUI_SLIDER_FIELD_TEXT_COLOR = UINT32_C(1) << 10,
    TINYUI_SLIDER_FIELD_BORDER_COLOR = UINT32_C(1) << 11,
    TINYUI_SLIDER_FIELD_RADIUS = UINT32_C(1) << 12,
    TINYUI_SLIDER_FIELD_PADDING = UINT32_C(1) << 13,
    TINYUI_SLIDER_FIELD_HORIZONTAL = UINT32_C(1) << 14,
    TINYUI_SLIDER_FIELD_BACKGROUND_SOURCE = UINT32_C(1) << 15,
    TINYUI_SLIDER_FIELD_INDICATOR_SOURCE = UINT32_C(1) << 16,
    TINYUI_SLIDER_FIELD_INDICATOR_WIDTH = UINT32_C(1) << 17,
    TINYUI_SLIDER_FIELD_SLIM_SIZE = UINT32_C(1) << 18,
} tinyui_slider_field_t;

typedef struct tinyui_slider_props {
    uint32_t fields;
    uint16_t id;
    int min_value;
    int max_value;
    int value;
    tinyui_value_changed_cb on_value_changed;
    void *user_data;
    const char *style_class;
    int width;
    int height;
    unsigned int bg_color;
    unsigned int text_color;
    unsigned int border_color;
    int radius;
    int padding;
    int horizontal;
    struct tinyui_image_source *background_source;
    struct tinyui_image_source *indicator_source;
    int indicator_width;
    int slim_size;
} tinyui_slider_props_t;

tinyui_obj_t *tinyui_slider_create(tinyui_obj_t *parent);

tinyui_obj_t *tinyui_slider_create_with_props(tinyui_obj_t *parent,
                                               const tinyui_slider_props_t *props);

int tinyui_slider_set_value(tinyui_obj_t *slider, int value);

int tinyui_slider_set_range(tinyui_obj_t *slider, int min_value, int max_value);

int tinyui_slider_set_percent(tinyui_obj_t *slider, int percent);

int tinyui_slider_set_horizontal(tinyui_obj_t *slider, int horizontal);

int tinyui_slider_get_horizontal(tinyui_obj_t *slider, int *horizontal);

int tinyui_slider_set_background_source(tinyui_obj_t *slider, struct tinyui_image_source *source);

int tinyui_slider_set_indicator_source(tinyui_obj_t *slider, struct tinyui_image_source *source);

int tinyui_slider_set_image(tinyui_obj_t *slider, struct tinyui_image_source *background_source, struct tinyui_image_source *indicator_source);

int tinyui_slider_set_color(tinyui_obj_t *slider, unsigned int bg_color, unsigned int frame_color, unsigned int indicator_color);

int tinyui_slider_set_indicator_width(tinyui_obj_t *slider, int indicator_width);

int tinyui_slider_set_slim_size(tinyui_obj_t *slider, int slim_size);

int tinyui_slider_get_percent(tinyui_obj_t *slider, int *percent);

int tinyui_slider_set_on_value_changed(tinyui_obj_t *slider, tinyui_value_changed_cb cb, void *user_data);

#endif /* TINYUI_SLIDER_H */
