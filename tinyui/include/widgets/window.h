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

#ifndef TINYUI_WINDOW_H
#define TINYUI_WINDOW_H

#include "tinyui_config.h"
#if !TINYUI_ENABLE_WINDOW
#  ifndef TINYUI_INTERNAL_FEATURE_HEADER
#  error "TINYUI_ENABLE_WINDOW is disabled"
#  endif
#endif

#include "core/obj.h"
#include <stdint.h>

struct tinyui_image_source;

enum tinyui_window_layout_type {
    TINYUI_WINDOW_LAYOUT_NONE,
    TINYUI_WINDOW_LAYOUT_FLEX,
    TINYUI_WINDOW_LAYOUT_GRID,
};

typedef enum tinyui_window_field {
    TINYUI_WINDOW_FIELD_ID = UINT32_C(1) << 0,
    TINYUI_WINDOW_FIELD_STYLE_CLASS = UINT32_C(1) << 1,
    TINYUI_WINDOW_FIELD_USER_DATA = UINT32_C(1) << 2,
    TINYUI_WINDOW_FIELD_BG_COLOR = UINT32_C(1) << 3,
    TINYUI_WINDOW_FIELD_TEXT_COLOR = UINT32_C(1) << 4,
    TINYUI_WINDOW_FIELD_BORDER_COLOR = UINT32_C(1) << 5,
    TINYUI_WINDOW_FIELD_RADIUS = UINT32_C(1) << 6,
    TINYUI_WINDOW_FIELD_PADDING = UINT32_C(1) << 7,
    TINYUI_WINDOW_FIELD_BACKGROUND_SOURCE = UINT32_C(1) << 8,
    TINYUI_WINDOW_FIELD_PADDING_LEFT = UINT32_C(1) << 9,
    TINYUI_WINDOW_FIELD_PADDING_TOP = UINT32_C(1) << 10,
    TINYUI_WINDOW_FIELD_PADDING_RIGHT = UINT32_C(1) << 11,
    TINYUI_WINDOW_FIELD_PADDING_BOTTOM = UINT32_C(1) << 12,
} tinyui_window_field_t;

struct tinyui_window_props {
    uint32_t fields;
    uint16_t id;
    const char *style_class;
    void *user_data;
    unsigned int bg_color;
    unsigned int text_color;
    unsigned int border_color;
    int radius;
    int padding;
    struct tinyui_image_source *background_source;
    int padding_left;
    int padding_top;
    int padding_right;
    int padding_bottom;
};
#ifndef TINYUI_WINDOW_PROPS_T_DEFINED
typedef struct tinyui_window_props tinyui_window_props_t;
#define TINYUI_WINDOW_PROPS_T_DEFINED
#endif

tinyui_obj_t *tinyui_window_create(tinyui_obj_t *parent);

tinyui_obj_t *tinyui_window_create_with_props(tinyui_obj_t *parent,
                                               const tinyui_window_props_t *props);

int tinyui_window_set_background_source(tinyui_obj_t *window, struct tinyui_image_source *source);

int tinyui_window_set_background_offset(tinyui_obj_t *window, int offset_x, int offset_y);

int tinyui_window_get_background_offset(tinyui_obj_t *window, int *offset_x, int *offset_y);

int tinyui_window_set_color(tinyui_obj_t *window, unsigned int rgb);

int tinyui_window_get_color(tinyui_obj_t *window, unsigned int *rgb);

int tinyui_window_set_layout_type(tinyui_obj_t *window, enum tinyui_window_layout_type type);

int tinyui_window_get_layout_type(tinyui_obj_t *window, enum tinyui_window_layout_type *type);

int tinyui_window_set_padding(tinyui_obj_t *window, int left, int top, int right, int bottom);

int tinyui_window_set_grid_padding(tinyui_obj_t *window, int left, int top, int right, int bottom);

int tinyui_window_set_gap(tinyui_obj_t *window, int gap);

int tinyui_window_get_gap(tinyui_obj_t *window);

int tinyui_window_get_padding_group(tinyui_obj_t *window, int *left, int *top, int *right, int *bottom);

#endif /* TINYUI_WINDOW_H */
