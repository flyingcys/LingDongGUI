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

#ifndef TINYUI_ICON_SLIDER_H
#define TINYUI_ICON_SLIDER_H

#include "tinyui_config.h"
#if !TINYUI_ENABLE_ICON_SLIDER
#  ifndef TINYUI_INTERNAL_FEATURE_HEADER
#  error "TINYUI_ENABLE_ICON_SLIDER is disabled"
#  endif
#endif

#include "core/obj.h"
#include <stdint.h>

struct tinyui_image_source;

typedef enum tinyui_icon_slider_field {
    TINYUI_ICON_SLIDER_FIELD_ID = UINT32_C(1) << 0,
    TINYUI_ICON_SLIDER_FIELD_STYLE_CLASS = UINT32_C(1) << 1,
    TINYUI_ICON_SLIDER_FIELD_USER_DATA = UINT32_C(1) << 2,
    TINYUI_ICON_SLIDER_FIELD_WIDTH = UINT32_C(1) << 3,
    TINYUI_ICON_SLIDER_FIELD_HEIGHT = UINT32_C(1) << 4,
    TINYUI_ICON_SLIDER_FIELD_ICON_WIDTH = UINT32_C(1) << 5,
    TINYUI_ICON_SLIDER_FIELD_ICON_SPACE = UINT32_C(1) << 6,
    TINYUI_ICON_SLIDER_FIELD_COLUMNS = UINT32_C(1) << 7,
    TINYUI_ICON_SLIDER_FIELD_ROWS = UINT32_C(1) << 8,
    TINYUI_ICON_SLIDER_FIELD_PAGES = UINT32_C(1) << 9,
    TINYUI_ICON_SLIDER_FIELD_HORIZONTAL = UINT32_C(1) << 10,
} tinyui_icon_slider_field_t;

typedef struct tinyui_icon_slider_props {
    uint32_t fields;
    uint16_t id;
    const char *style_class;
    void *user_data;
    int width;
    int height;
    int icon_width;
    int icon_space;
    int columns;
    int rows;
    int pages;
    int horizontal;
} tinyui_icon_slider_props_t;

tinyui_obj_t *tinyui_icon_slider_create(tinyui_obj_t *parent);

tinyui_obj_t *tinyui_icon_slider_create_with_props(tinyui_obj_t *parent,
                                               const tinyui_icon_slider_props_t *props);

int tinyui_icon_slider_add_item(tinyui_obj_t *icon_slider, const char *id, const char *text);

int tinyui_icon_slider_add_item_with_source(tinyui_obj_t *icon_slider, const char *id, const char *text, struct tinyui_image_source *source);

int tinyui_icon_slider_add_icon(tinyui_obj_t *icon_slider, const char *id, const char *text, struct tinyui_image_source *source);

int tinyui_icon_slider_set_layout(tinyui_obj_t *icon_slider, int width, int height, int icon_width, int icon_space, int columns, int rows, int pages);

int tinyui_icon_slider_set_selected_index(tinyui_obj_t *icon_slider, int index);

int tinyui_icon_slider_get_selected_index(const tinyui_obj_t *icon_slider);

int tinyui_icon_slider_set_horizontal(tinyui_obj_t *icon_slider, int horizontal);

int tinyui_icon_slider_set_horizontal_scroll(tinyui_obj_t *icon_slider, int horizontal);

int tinyui_icon_slider_get_horizontal(const tinyui_obj_t *icon_slider, int *horizontal);

int tinyui_icon_slider_set_speed(tinyui_obj_t *icon_slider, int speed);

void tinyui_icon_slider_set_on_selected(tinyui_obj_t *icon_slider, void (*callback)(tinyui_obj_t *icon_slider, int index, void *user_data), void *user_data);

#endif /* TINYUI_ICON_SLIDER_H */
