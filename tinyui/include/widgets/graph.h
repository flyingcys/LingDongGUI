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

#ifndef TINYUI_GRAPH_H
#define TINYUI_GRAPH_H

#include "core/obj.h"
#include <stdint.h>

struct tinyui_image_source;

typedef enum tinyui_graph_field {
    TINYUI_GRAPH_FIELD_ID = UINT32_C(1) << 0,
    TINYUI_GRAPH_FIELD_SERIES_MAX = UINT32_C(1) << 1,
    TINYUI_GRAPH_FIELD_WIDTH = UINT32_C(1) << 2,
    TINYUI_GRAPH_FIELD_HEIGHT = UINT32_C(1) << 3,
    TINYUI_GRAPH_FIELD_STYLE_CLASS = UINT32_C(1) << 4,
    TINYUI_GRAPH_FIELD_USER_DATA = UINT32_C(1) << 5,
} tinyui_graph_field_t;

typedef struct tinyui_graph_props {
    uint32_t fields;
    uint16_t id;
    int series_max;
    int width;
    int height;
    const char *style_class;
    void *user_data;
} tinyui_graph_props_t;

tinyui_obj_t *tinyui_graph_create(tinyui_obj_t *parent);

tinyui_obj_t *tinyui_graph_create_with_props(tinyui_obj_t *parent,
                                               const tinyui_graph_props_t *props);

int tinyui_graph_set_axis(tinyui_obj_t *graph, int x_axis, int y_axis);

int tinyui_graph_set_axis_offset(tinyui_obj_t *graph, int axis_offset);

int tinyui_graph_set_frame_space(tinyui_obj_t *graph, int frame_space);

int tinyui_graph_set_grid_offset(tinyui_obj_t *graph, int grid_offset);

int tinyui_graph_set_point_image_mask(tinyui_obj_t *graph, struct tinyui_image_source *source);

int tinyui_graph_set_point_mask_source(tinyui_obj_t *graph, struct tinyui_image_source *source);

int tinyui_graph_add_series(tinyui_obj_t *graph, unsigned int series_color, int line_size, int point_max);

int tinyui_graph_set_value(tinyui_obj_t *graph, int series_index, int value_index, int value);

int tinyui_graph_move_add(tinyui_obj_t *graph, int series_index, int value);

int tinyui_graph_get_series_count(const tinyui_obj_t *graph);

int tinyui_graph_get_value(const tinyui_obj_t *graph, int series_index, int value_index);

#endif /* TINYUI_GRAPH_H */
