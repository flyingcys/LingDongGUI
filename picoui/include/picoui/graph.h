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

#ifndef PICOUI_GRAPH_H
#define PICOUI_GRAPH_H

#include "picoui/widget.h"

struct picoui_window;
struct picoui_graph;

struct picoui_graph_props {
    const char *id;
    int series_max;
    int width;
    int height;
    const char *style_class;
    void *user_data;
};

/**
 * @brief Create graph widget
 *
 * @param[in] parent Parent widget
 * @param[in] id Widget identifier string
 * @param[in] series_max series max
 * @return Pointer to the object on success, NULL on failure
 */

struct picoui_graph *picoui_graph_create(struct picoui_window *parent,
                                         const char *id,
                                         int series_max);

/**
 * @brief Create graph widget with properties
 *
 * @param[in] parent Parent widget
 * @param[in] props Properties structure
 * @return Pointer to the object on success, NULL on failure
 */

struct picoui_graph *picoui_graph_create_with_props(struct picoui_window *parent,
                                                    const struct picoui_graph_props *props);

/**
 * @brief graph init
 *
 * @param[in] parent Parent widget
 * @param[in] id Widget identifier string
 * @param[in] series_max series max
 * @return Pointer to the object on success, NULL on failure
 */

struct picoui_graph *picoui_graph_init(struct picoui_window *parent,
                                       const char *id,
                                       int series_max);

/**
 * @brief Set axis of graph widget
 *
 * @param[in] graph Graph widget instance
 * @param[in] x_axis x axis
 * @param[in] y_axis y axis
 * @return 0 on success, -1 on failure
 */

int picoui_graph_set_axis(struct picoui_graph *graph, int x_axis, int y_axis);

/**
 * @brief Set axis offset of graph widget
 *
 * @param[in] graph Graph widget instance
 * @param[in] axis_offset axis offset
 * @return 0 on success, -1 on failure
 */

int picoui_graph_set_axis_offset(struct picoui_graph *graph, int axis_offset);

/**
 * @brief Set frame space of graph widget
 *
 * @param[in] graph Graph widget instance
 * @param[in] frame_space frame space
 * @return 0 on success, -1 on failure
 */

int picoui_graph_set_frame_space(struct picoui_graph *graph, int frame_space);

/**
 * @brief Set grid offset of graph widget
 *
 * @param[in] graph Graph widget instance
 * @param[in] grid_offset grid offset
 * @return 0 on success, -1 on failure
 */

int picoui_graph_set_grid_offset(struct picoui_graph *graph, int grid_offset);

/**
 * @brief Set point image mask of graph widget
 *
 * @param[in] graph Graph widget instance
 * @param[in] source Image source
 * @return 0 on success, -1 on failure
 */

int picoui_graph_set_point_image_mask(struct picoui_graph *graph,
                                      struct picoui_image_source *source);

/**
 * @brief Set point mask source of graph widget
 *
 * @param[in] graph Graph widget instance
 * @param[in] source Image source
 * @return 0 on success, -1 on failure
 */

int picoui_graph_set_point_mask_source(struct picoui_graph *graph,
                                       struct picoui_image_source *source);

/**
 * @brief graph add series
 *
 * @param[in] graph Graph widget instance
 * @param[in] series_color series color
 * @param[in] line_size line size
 * @param[in] point_max point max
 * @return 0 on success, -1 on failure
 */

int picoui_graph_add_series(struct picoui_graph *graph,
                            unsigned int series_color,
                            int line_size,
                            int point_max);

/**
 * @brief Set value of graph widget
 *
 * @param[in] graph Graph widget instance
 * @param[in] series_index series index
 * @param[in] value_index value index
 * @param[in] value Value
 * @return 0 on success, -1 on failure
 */

int picoui_graph_set_value(struct picoui_graph *graph,
                           int series_index,
                           int value_index,
                           int value);

/**
 * @brief graph move add
 *
 * @param[in] graph Graph widget instance
 * @param[in] series_index series index
 * @param[in] value Value
 * @return 0 on success, -1 on failure
 */

int picoui_graph_move_add(struct picoui_graph *graph, int series_index, int value);

/**
 * @brief Get series count of graph widget
 *
 * @param[in] graph Graph widget instance
 * @return The property value, negative on error
 */

int picoui_graph_get_series_count(const struct picoui_graph *graph);

/**
 * @brief Get value of graph widget
 *
 * @param[in] graph Graph widget instance
 * @param[in] series_index series index
 * @param[in] value_index value index
 * @return The property value, negative on error
 */

int picoui_graph_get_value(const struct picoui_graph *graph, int series_index, int value_index);

#endif
