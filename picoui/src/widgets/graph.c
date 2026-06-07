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

#include "internal.h"
#include "picoui/graph.h"

#include <stdlib.h>
#include <string.h>

struct picoui_graph_ext {
    struct picoui_graph graph;
    int values[PICOUI_GRAPH_MAX_SERIES][PICOUI_GRAPH_MAX_POINTS];
    int rendered_series_count;
    int rendered_series_point_counts[PICOUI_GRAPH_MAX_SERIES];
    int rendered_values[PICOUI_GRAPH_MAX_SERIES][PICOUI_GRAPH_MAX_POINTS];
    int render_ready;
};

static struct picoui_graph_ext *picoui_graph_ext_from_graph(struct picoui_graph *graph)
{
    if (graph == 0) {
        return 0;
    }

    return (struct picoui_graph_ext *)graph;
}

static const struct picoui_graph_ext *picoui_graph_ext_from_graph_const(const struct picoui_graph *graph)
{
    if (graph == 0) {
        return 0;
    }

    return (const struct picoui_graph_ext *)graph;
}

static int picoui_graph_props_are_valid(const struct picoui_graph_props *props)
{
    return props != 0 &&
           props->id != 0 &&
           props->series_max > 0 &&
           props->series_max <= PICOUI_GRAPH_MAX_SERIES &&
           props->width >= 0 &&
           props->height >= 0;
}

static int picoui_graph_apply_native_geometry(struct picoui_graph *graph)
{
    if (graph == 0) {
        return -1;
    }

    if (picoui_backend_graph_set_frame_space(graph->widget.backend_widget, graph->frame_space) != 0) {
        return -1;
    }
    if (picoui_backend_graph_set_axis(graph->widget.backend_widget, graph->x_axis, graph->y_axis) != 0) {
        return -1;
    }
    if (picoui_backend_graph_set_axis_offset(graph->widget.backend_widget, graph->axis_offset) != 0) {
        return -1;
    }
    if (picoui_backend_graph_set_grid_offset(graph->widget.backend_widget, graph->grid_offset) != 0) {
        return -1;
    }
    if (graph->point_mask_source != 0 &&
        picoui_backend_graph_set_point_mask_source(graph->widget.backend_widget, graph->point_mask_source) != 0) {
        return -1;
    }

    return 0;
}

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
                                         int series_max)
{
    struct picoui_graph *graph;
    struct picoui_graph_ext *ext;

    if (parent == 0 || id == 0 || series_max <= 0 || series_max > PICOUI_GRAPH_MAX_SERIES) {
        return 0;
    }

    ext = calloc(1, sizeof(*ext));
    if (ext == 0) {
        return 0;
    }
    graph = &ext->graph;

    graph->widget.backend_widget =
        picoui_backend_create_graph(parent->widget.backend_widget, id, series_max);
    if (graph->widget.backend_widget == 0) {
        free(ext);
        return 0;
    }

    graph->id = id;
    graph->series_max = series_max;
    graph->x_axis = 100;
    graph->y_axis = 100;
    graph->axis_offset = 5;
    graph->frame_space = 8;
    graph->grid_offset = 20;
    graph->widget.visible = 1;
    graph->widget.enabled = 1;
    if (picoui_backend_widget_bind_host(graph->widget.backend_widget, &graph->widget) != 0) {
        free(ext);
        return 0;
    }
    return graph;
}

/**
 * @brief graph init
 *
 * @param[in] parent Parent widget
 * @param[in] id Widget identifier string
 * @param[in] series_max series max
 * @return Pointer to the object
 */

struct picoui_graph *picoui_graph_init(struct picoui_window *parent,
                                       const char *id,
                                       int series_max)
{
    return picoui_graph_create(parent, id, series_max);
}

/**
 * @brief Create graph widget with properties
 *
 * @param[in] parent Parent widget
 * @param[in] props Properties structure
 * @return Pointer to the object on success, NULL on failure
 */

struct picoui_graph *picoui_graph_create_with_props(struct picoui_window *parent,
                                                    const struct picoui_graph_props *props)
{
    struct picoui_graph *graph;

    if (!picoui_graph_props_are_valid(props)) {
        return 0;
    }

    graph = picoui_graph_create(parent, props->id, props->series_max);
    if (graph == 0) {
        return 0;
    }

    if (picoui_widget_set_user_data(&graph->widget, props->user_data) != 0) {
        free(graph);
        return 0;
    }
    if (props->style_class != 0 &&
        picoui_widget_set_style_class(&graph->widget, props->style_class) != 0) {
        free(graph);
        return 0;
    }
    if ((props->width > 0 || props->height > 0) &&
        picoui_widget_set_size(&graph->widget, props->width, props->height) != 0) {
        free(graph);
        return 0;
    }

    return graph;
}

/**
 * @brief Set axis of graph widget
 *
 * @param[in] graph Graph widget instance
 * @param[in] x_axis x axis
 * @param[in] y_axis y axis
 * @return -1 on failure
 */

int picoui_graph_set_axis(struct picoui_graph *graph, int x_axis, int y_axis)
{
    if (graph == 0 || x_axis <= 0 || y_axis <= 0) {
        return -1;
    }

    graph->x_axis = x_axis;
    graph->y_axis = y_axis;
    return picoui_graph_apply_native_geometry(graph);
}

/**
 * @brief Set axis offset of graph widget
 *
 * @param[in] graph Graph widget instance
 * @param[in] axis_offset axis offset
 * @return -1 on failure
 */

int picoui_graph_set_axis_offset(struct picoui_graph *graph, int axis_offset)
{
    if (graph == 0 || axis_offset < 0) {
        return -1;
    }

    graph->axis_offset = axis_offset;
    return picoui_graph_apply_native_geometry(graph);
}

/**
 * @brief Set frame space of graph widget
 *
 * @param[in] graph Graph widget instance
 * @param[in] frame_space frame space
 * @return -1 on failure
 */

int picoui_graph_set_frame_space(struct picoui_graph *graph, int frame_space)
{
    if (graph == 0 || frame_space < 0) {
        return -1;
    }

    graph->frame_space = frame_space;
    return picoui_graph_apply_native_geometry(graph);
}

/**
 * @brief Set grid offset of graph widget
 *
 * @param[in] graph Graph widget instance
 * @param[in] grid_offset grid offset
 * @return -1 on failure
 */

int picoui_graph_set_grid_offset(struct picoui_graph *graph, int grid_offset)
{
    if (graph == 0 || grid_offset <= 0) {
        return -1;
    }

    graph->grid_offset = grid_offset;
    return picoui_graph_apply_native_geometry(graph);
}

/**
 * @brief Set point mask source of graph widget
 *
 * @param[in] graph Graph widget instance
 * @param[in] source Image source
 * @return -1 on failure
 */

int picoui_graph_set_point_mask_source(struct picoui_graph *graph, struct picoui_image_source *source)
{
    if (graph == 0 || source == 0 || source->mask_tile == 0) {
        return -1;
    }

    graph->point_mask_source = source;
    return picoui_graph_apply_native_geometry(graph);
}

/**
 * @brief Set point image mask of graph widget
 *
 * @param[in] graph Graph widget instance
 * @param[in] source Image source
 * @return 0 on success, -1 on failure
 */

int picoui_graph_set_point_image_mask(struct picoui_graph *graph, struct picoui_image_source *source)
{
    return picoui_graph_set_point_mask_source(graph, source);
}

/**
 * @brief graph add series
 *
 * @param[in] graph Graph widget instance
 * @param[in] series_color series color
 * @param[in] line_size line size
 * @param[in] point_max point max
 * @return -1 on failure
 */

int picoui_graph_add_series(struct picoui_graph *graph,
                            unsigned int series_color,
                            int line_size,
                            int point_max)
{
    struct picoui_graph_ext *ext;
    int expected_index;
    int series_index;

    if (graph == 0 || line_size < 0 || point_max <= 0 || point_max > PICOUI_GRAPH_MAX_POINTS ||
        graph->series_count >= graph->series_max) {
        return -1;
    }

    ext = picoui_graph_ext_from_graph(graph);
    if (ext == 0) {
        return -1;
    }

    expected_index = graph->series_count;
    series_index = picoui_backend_graph_add_series(graph->widget.backend_widget,
                                                   series_color,
                                                   line_size,
                                                   point_max);
    if (series_index < 0 || series_index >= PICOUI_GRAPH_MAX_SERIES || series_index != expected_index) {
        return -1;
    }

    graph->series_point_counts[series_index] = point_max;
    memset(ext->values[series_index], 0, sizeof(ext->values[series_index]));
    graph->series_count = series_index + 1;
    return series_index;
}

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
                           int value)
{
    struct picoui_graph_ext *ext;
    int old_value;

    if (graph == 0 || series_index < 0 || series_index >= graph->series_count ||
        value_index < 0 || value_index >= graph->series_point_counts[series_index] ||
        value < 0) {
        return -1;
    }

    ext = picoui_graph_ext_from_graph(graph);
    if (ext == 0) {
        return -1;
    }

    old_value = ext->values[series_index][value_index];
    ext->values[series_index][value_index] = value;
    if (picoui_backend_graph_set_value(graph->widget.backend_widget, series_index, value_index, value) != 0) {
        ext->values[series_index][value_index] = old_value;
        return -1;
    }
    return 0;
}

/**
 * @brief graph move add
 *
 * @param[in] graph Graph widget instance
 * @param[in] series_index series index
 * @param[in] value Value
 * @return -1 on failure
 */

int picoui_graph_move_add(struct picoui_graph *graph, int series_index, int value)
{
    struct picoui_graph_ext *ext;
    int point_count;
    int old_values[PICOUI_GRAPH_MAX_POINTS];

    if (graph == 0 || series_index < 0 || series_index >= graph->series_count || value < 0) {
        return -1;
    }

    ext = picoui_graph_ext_from_graph(graph);
    if (ext == 0) {
        return -1;
    }

    point_count = graph->series_point_counts[series_index];
    if (point_count <= 0 || point_count > PICOUI_GRAPH_MAX_POINTS) {
        return -1;
    }

    memcpy(old_values, ext->values[series_index], (size_t)point_count * sizeof(int));
    if (point_count > 1) {
        memmove(ext->values[series_index],
                ext->values[series_index] + 1,
                (size_t)(point_count - 1) * sizeof(int));
    }
    ext->values[series_index][point_count - 1] = value;

    if (picoui_backend_graph_move_add(graph->widget.backend_widget, series_index, value) != 0) {
        memcpy(ext->values[series_index], old_values, (size_t)point_count * sizeof(int));
        return -1;
    }

    return 0;
}

/**
 * @brief Get series count of graph widget
 *
 * @param[in] graph Graph widget instance
 * @return -1 on failure
 */

int picoui_graph_get_series_count(const struct picoui_graph *graph)
{
    if (graph == 0) {
        return -1;
    }

    return graph->series_count;
}

/**
 * @brief Get value of graph widget
 *
 * @param[in] graph Graph widget instance
 * @param[in] series_index series index
 * @param[in] value_index value index
 * @return -1 on failure
 */

int picoui_graph_get_value(const struct picoui_graph *graph, int series_index, int value_index)
{
    const struct picoui_graph_ext *ext;

    if (graph == 0 || series_index < 0 || series_index >= graph->series_count ||
        value_index < 0 || value_index >= graph->series_point_counts[series_index]) {
        return -1;
    }

    ext = picoui_graph_ext_from_graph_const(graph);
    if (ext == 0) {
        return -1;
    }

    return ext->values[series_index][value_index];
}
