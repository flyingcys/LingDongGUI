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

#include "backend.h"
#include "internal.h"
#include "runtime_bridge.h"
#include "ldGraph.h"

#include <stdlib.h>

extern const arm_2d_tile_t c_tileWhiteDotMask;

static ldGraph_t *picoui_backend_graph_get_ld(void *backend_widget)
{
    struct picoui_backend_widget *widget = backend_widget;

    if (widget == NULL || widget->ld_widget == NULL) {
        return NULL;
    }

    return (ldGraph_t *)widget->ld_widget;
}

/**
 * @brief Create backend for graph
 *
 * @param[in] parent Parent widget
 * @param[in] id Widget identifier string
 * @param[in] series_max series max
 */

void *picoui_backend_create_graph(void *parent, const char *id, int series_max)
{
    struct picoui_backend_widget *widget;
    struct picoui_backend_widget *parent_widget = parent;
    struct picoui_backend_app_state *app_state;
    ldGraph_t *ld_graph;
    uint16_t name_id;

    if (parent == 0 || id == 0 || series_max <= 0 || series_max > PICOUI_GRAPH_MAX_SERIES) {
        return 0;
    }

    app_state = picoui_runtime_bridge_backend_state_from_parent(parent);
    if (app_state == NULL || app_state->ld_scene == NULL || parent_widget->ld_widget == NULL) {
        return 0;
    }

    widget = calloc(1, sizeof(*widget));
    if (widget == 0) {
        return 0;
    }

    name_id = picoui_runtime_bridge_next_name_id(parent);
    if (name_id == 0) {
        free(widget);
        return 0;
    }
    ld_graph = ldGraph_init(app_state->ld_scene,
                            NULL,
                            name_id,
                            parent_widget->ld_name_id,
                            0,
                            0,
                            240,
                            120,
                            (uint8_t)series_max);
    if (ld_graph == NULL) {
        free(widget);
        return 0;
    }

    ldGraphSetFrameSpace(ld_graph, 8, false);
    ldGraphSetGridOffset(ld_graph, 20);
    ldGraphSetAxis(ld_graph, 100, 100, 5);
    ldGraphSetPointImageMask(ld_graph, (arm_2d_tile_t *)&c_tileWhiteDotMask);

    if (picoui_backend_widget_init_child(widget,
                                         parent,
                                         PICOUI_BACKEND_WIDGET_GRAPH,
                                         id,
                                         parent_widget->theme) != 0) {
        ldGraph_depose(app_state->ld_scene, ld_graph);
        free(widget);
        return 0;
    }
    widget->ld_widget = ld_graph;
    widget->ld_name_id = name_id;
    if (picoui_backend_widget_attach_child(parent, widget) != 0) {
        ldGraph_depose(app_state->ld_scene, ld_graph);
        free(widget);
        return 0;
    }
    return widget;
}

/**
 * @brief Set axis of graph backend
 *
 * @param[in] backend_widget backend widget
 * @param[in] x_axis x axis
 * @param[in] y_axis y axis
 * @return 0 on success, -1 on failure
 */

int picoui_backend_graph_set_axis(void *backend_widget, int x_axis, int y_axis)
{
    ldGraph_t *ld_graph = picoui_backend_graph_get_ld(backend_widget);

    if (ld_graph == NULL || x_axis <= 0 || y_axis <= 0) {
        return -1;
    }

    ldGraphSetAxis(ld_graph, (uint16_t)x_axis, (uint16_t)y_axis, ld_graph->xAxisOffset);
    return 0;
}

/**
 * @brief Set axis offset of graph backend
 *
 * @param[in] backend_widget backend widget
 * @param[in] axis_offset axis offset
 * @return 0 on success, -1 on failure
 */

int picoui_backend_graph_set_axis_offset(void *backend_widget, int axis_offset)
{
    ldGraph_t *ld_graph = picoui_backend_graph_get_ld(backend_widget);

    if (ld_graph == NULL || axis_offset < 0) {
        return -1;
    }

    ldGraphSetAxisOffset(ld_graph, (uint16_t)axis_offset);
    return 0;
}

/**
 * @brief Set frame space of graph backend
 *
 * @param[in] backend_widget backend widget
 * @param[in] frame_space frame space
 * @return 0 on success, -1 on failure
 */

int picoui_backend_graph_set_frame_space(void *backend_widget, int frame_space)
{
    ldGraph_t *ld_graph = picoui_backend_graph_get_ld(backend_widget);

    if (ld_graph == NULL || frame_space < 0) {
        return -1;
    }

    ldGraphSetFrameSpace(ld_graph, (uint8_t)frame_space, false);
    return 0;
}

/**
 * @brief Set grid offset of graph backend
 *
 * @param[in] backend_widget backend widget
 * @param[in] grid_offset grid offset
 * @return 0 on success, -1 on failure
 */

int picoui_backend_graph_set_grid_offset(void *backend_widget, int grid_offset)
{
    ldGraph_t *ld_graph = picoui_backend_graph_get_ld(backend_widget);

    if (ld_graph == NULL || grid_offset <= 0) {
        return -1;
    }

    ldGraphSetGridOffset(ld_graph, (uint8_t)grid_offset);
    return 0;
}

/**
 * @brief Set point mask source of graph backend
 *
 * @param[in] backend_widget backend widget
 * @param[in] source Image source
 * @return 0 on success, -1 on failure
 */

int picoui_backend_graph_set_point_mask_source(void *backend_widget, struct picoui_image_source *source)
{
    ldGraph_t *ld_graph = picoui_backend_graph_get_ld(backend_widget);

    if (ld_graph == NULL || source == NULL || source->mask_tile == NULL) {
        return -1;
    }

    ldGraphSetPointImageMask(ld_graph, source->mask_tile);
    return 0;
}

/**
 * @brief Add_ series from graph
 *
 * @param[in] backend_widget backend widget
 * @param[in] series_color series color
 * @param[in] line_size line size
 * @param[in] point_max point max
 * @return -1 on failure
 */

int picoui_backend_graph_add_series(void *backend_widget,
                                    unsigned int series_color,
                                    int line_size,
                                    int point_max)
{
    ldGraph_t *ld_graph = picoui_backend_graph_get_ld(backend_widget);

    if (ld_graph == NULL || line_size < 0 || point_max <= 0 || point_max > PICOUI_GRAPH_MAX_POINTS) {
        return -1;
    }

    return (int)ldGraphAddSeries(ld_graph, (ldColor)series_color, (uint8_t)line_size, (uint16_t)point_max);
}

/**
 * @brief Set value of graph backend
 *
 * @param[in] backend_widget backend widget
 * @param[in] series_index series index
 * @param[in] value_index value index
 * @param[in] value Value
 * @return 0 on success, -1 on failure
 */

int picoui_backend_graph_set_value(void *backend_widget, int series_index, int value_index, int value)
{
    ldGraph_t *ld_graph = picoui_backend_graph_get_ld(backend_widget);

    if (ld_graph == NULL || series_index < 0 || series_index >= ld_graph->seriesCount ||
        value_index < 0 || value_index >= ld_graph->pSeries[series_index].valueCountMax ||
        value < 0) {
        return -1;
    }

    ldGraphSetValue(ld_graph, (uint8_t)series_index, (uint16_t)value_index, (uint16_t)value);
    return 0;
}

/**
 * @brief graph: move add
 *
 * @param[in] backend_widget backend widget
 * @param[in] series_index series index
 * @param[in] value Value
 * @return 0 on success, -1 on failure
 */

int picoui_backend_graph_move_add(void *backend_widget, int series_index, int value)
{
    ldGraph_t *ld_graph = picoui_backend_graph_get_ld(backend_widget);

    if (ld_graph == NULL || series_index < 0 || series_index >= ld_graph->seriesCount || value < 0) {
        return -1;
    }

    ldGraphMoveAdd(ld_graph, (uint8_t)series_index, (uint16_t)value);
    return 0;
}

/**
 * @brief Get series count from graph backend
 *
 * @param[in] backend_widget backend widget
 * @return -1 on failure
 */

int picoui_backend_graph_get_series_count(void *backend_widget)
{
    ldGraph_t *ld_graph = picoui_backend_graph_get_ld(backend_widget);

    if (ld_graph == NULL) {
        return -1;
    }

    return (int)ld_graph->seriesCount;
}

/**
 * @brief Get value from graph backend
 *
 * @param[in] backend_widget backend widget
 * @param[in] series_index series index
 * @param[in] value_index value index
 * @return -1 on failure
 */

int picoui_backend_graph_get_value(void *backend_widget, int series_index, int value_index)
{
    ldGraph_t *ld_graph = picoui_backend_graph_get_ld(backend_widget);

    if (ld_graph == NULL || series_index < 0 || series_index >= ld_graph->seriesCount ||
        value_index < 0 || value_index >= ld_graph->pSeries[series_index].valueCountMax) {
        return -1;
    }

    return (int)ld_graph->pSeries[series_index].pValueList[value_index];
}
