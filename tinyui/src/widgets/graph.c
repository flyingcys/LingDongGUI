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
#include "graph.h"
#include "../core/runtime_bridge.h"
#include "../../../src/gui/ldGraph.h"

#include <stdlib.h>

/* ---- C2 depose closure state (file-static, single-threaded scope) ---- */
static ld_scene_t *s_graph_depose_scene = NULL;

static void tinyui_graph_ld_depose_cb(void *ld_widget)
{
    if (s_graph_depose_scene != NULL) {
        ldGraph_depose(s_graph_depose_scene, (ldGraph_t *)ld_widget);
        s_graph_depose_scene = NULL;
    }
}

static int graph_props_valid(const struct tinyui_graph_props *props)
{
    return props != 0 &&
           props->id != 0 &&
           props->series_max > 0 &&
           props->series_max <= TINYUI_GRAPH_MAX_SERIES &&
           props->width >= 0 &&
           props->height >= 0;
}

struct tinyui_graph_create_ctx {
    int series_max;
};

static void *tinyui_graph_ld_init(void *ctx,
                                  struct ld_scene_t *scene,
                                  uint16_t name_id,
                                  uint16_t parent_name_id)
{
    const struct tinyui_graph_create_ctx *create_ctx =
        (const struct tinyui_graph_create_ctx *)ctx;
    ldGraph_t *ld_graph;

    if (create_ctx == 0) {
        return 0;
    }

    ld_graph = ldGraph_init(scene,
                            NULL,
                            name_id,
                            parent_name_id,
                            0,
                            0,
                            240,
                            120,
                            (uint8_t)create_ctx->series_max);
    if (ld_graph == 0) {
        return 0;
    }

    ldGraphSetFrameSpace(ld_graph, 8, false);
    ldGraphSetGridOffset(ld_graph, 20);
    ldGraphSetAxis(ld_graph, 100, 100, 5);
    ldGraphSetPointImageMask(ld_graph, (arm_2d_tile_t *)&c_tileWhiteDotMask);

    return ld_graph;
}

static int graph_apply_native_geometry_candidate(struct tinyui_graph *graph,
                                                  int x_axis,
                                                  int y_axis,
                                                  int axis_offset,
                                                  int frame_space,
                                                  int grid_offset,
                                                  struct tinyui_image_source *point_mask_source)
{
    struct tinyui_graph_native_snapshot {
        uint16_t x_axis_max;
        uint16_t y_axis_max;
        uint16_t x_axis_offset;
        float x_scale;
        float y_scale;
        arm_2d_tile_t *point_mask_tile;
        uint8_t frame_space;
        uint8_t grid_offset;
        bool is_frame;
        bool is_corner;
    } snapshot;
    ldGraph_t *ld_graph;
    arm_2d_tile_t *point_mask_tile;
    int effective_frame_space;
    int point_mask_width = 0;

    if (graph == 0) {
        return -1;
    }
    if (graph->widget.ld_widget == 0
        || graph->widget.kind != TINYUI_BACKEND_WIDGET_GRAPH) {
        return -1;
    }
    ld_graph = (ldGraph_t *)graph->widget.ld_widget;

    snapshot.x_axis_max = ld_graph->xAxisMax;
    snapshot.y_axis_max = ld_graph->yAxisMax;
    snapshot.x_axis_offset = ld_graph->xAxisOffset;
    snapshot.x_scale = ld_graph->xScale;
    snapshot.y_scale = ld_graph->yScale;
    snapshot.point_mask_tile = ld_graph->ptPointMaskTile;
    snapshot.frame_space = ld_graph->frameSpace;
    snapshot.grid_offset = ld_graph->gridOffset;
    snapshot.is_frame = ld_graph->isFrame;
    snapshot.is_corner = ld_graph->use_as__ldBase_t.isCorner;

    effective_frame_space = frame_space;
    if (point_mask_source != 0 && point_mask_source->mask_tile != 0) {
        point_mask_tile = (arm_2d_tile_t *)point_mask_source->mask_tile;
        point_mask_width = point_mask_tile->tRegion.tSize.iWidth;
        if (effective_frame_space < point_mask_width) {
            effective_frame_space = point_mask_width;
        }
    } else {
        point_mask_tile = 0;
    }

    ldGraphSetFrameSpace(ld_graph, (uint8_t)effective_frame_space, false);
    ldGraphSetAxis(ld_graph, (uint16_t)x_axis, (uint16_t)y_axis, ld_graph->xAxisOffset);
    ldGraphSetAxisOffset(ld_graph, (uint16_t)axis_offset);
    ldGraphSetGridOffset(ld_graph, (uint8_t)grid_offset);
    if (point_mask_source != 0) {
        ldGraphSetPointImageMask(ld_graph, point_mask_tile);
    } else if (ld_graph->ptPointMaskTile == 0) {
        ldGraphSetPointImageMask(ld_graph, (arm_2d_tile_t *)&c_tileWhiteDotMask);
    }

    if (ld_graph->frameSpace != (uint8_t)effective_frame_space) {
        goto fail;
    }
    if (ld_graph->gridOffset != (uint8_t)grid_offset) {
        goto fail;
    }
    if (ld_graph->xAxisOffset != (uint16_t)axis_offset) {
        goto fail;
    }
    if (point_mask_source != 0 && ld_graph->ptPointMaskTile != point_mask_tile) {
        goto fail;
    }
    if (ld_graph->xScale <= 0.0f || ld_graph->yScale <= 0.0f) {
        goto fail;
    }

    graph->x_axis = x_axis;
    graph->y_axis = y_axis;
    graph->axis_offset = axis_offset;
    graph->frame_space = effective_frame_space;
    graph->grid_offset = grid_offset;
    graph->point_mask_source = point_mask_source;

    return 0;

fail:
    ld_graph->xAxisMax = snapshot.x_axis_max;
    ld_graph->yAxisMax = snapshot.y_axis_max;
    ld_graph->xAxisOffset = snapshot.x_axis_offset;
    ld_graph->xScale = snapshot.x_scale;
    ld_graph->yScale = snapshot.y_scale;
    ld_graph->ptPointMaskTile = snapshot.point_mask_tile;
    ld_graph->frameSpace = snapshot.frame_space;
    ld_graph->gridOffset = snapshot.grid_offset;
    ld_graph->isFrame = snapshot.is_frame;
    ld_graph->use_as__ldBase_t.isCorner = snapshot.is_corner;
    return -1;
}

/**
 * @brief Create graph widget
 *
 * @param[in] parent Parent widget
 * @param[in] id Widget identifier string
 * @param[in] series_max series max
 * @return Pointer to the object on success, NULL on failure
 */

struct tinyui_graph *tinyui_graph_create(struct tinyui_window *parent,
                                         const char *id,
                                         int series_max)
{
    struct tinyui_graph *graph;
    struct tinyui_graph_create_ctx create_ctx;

    if (parent == 0 || id == 0 || series_max <= 0 || series_max > TINYUI_GRAPH_MAX_SERIES) {
        return 0;
    }

    if (parent->widget.owner == 0
        || parent->widget.owner->ld_scene == 0
        || parent->widget.ld_widget == 0) {
        return 0;
    }

    create_ctx.series_max = series_max;
    graph = (struct tinyui_graph *)tinyui_widget_create_leaf(&parent->widget,
                                                             TINYUI_BACKEND_WIDGET_GRAPH,
                                                             tinyui_graph_ld_init,
                                                             &create_ctx,
                                                             sizeof(*graph));
    if (graph == 0) {
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
    return graph;
}

/**
 * @brief Create graph widget with properties
 *
 * @param[in] parent Parent widget
 * @param[in] props Properties structure
 * @return Pointer to the object on success, NULL on failure
 */

struct tinyui_graph *tinyui_graph_create_with_props(struct tinyui_window *parent,
                                                    const struct tinyui_graph_props *props)
{
    struct tinyui_graph *graph;

    if (!graph_props_valid(props)) {
        return 0;
    }

    graph = tinyui_graph_create(parent, props->id, props->series_max);
    if (graph == 0) {
        return 0;
    }

    if (tinyui_widget_set_user_data(&graph->widget, props->user_data) != 0
        || (props->style_class != 0
            && tinyui_widget_set_style_class(&graph->widget, props->style_class) != 0)
        || ((props->width > 0 || props->height > 0)
            && tinyui_widget_set_size(&graph->widget, props->width, props->height) != 0)) {
        s_graph_depose_scene = graph->widget.ld_event_bridge_scene;
        tinyui_widget_destroy_common(&graph->widget, tinyui_graph_ld_depose_cb);
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

int tinyui_graph_set_axis(struct tinyui_graph *graph, int x_axis, int y_axis)
{
    if (graph == 0 || x_axis <= 0 || y_axis <= 0) {
        return -1;
    }

    return graph_apply_native_geometry_candidate(graph,
                                                  x_axis,
                                                  y_axis,
                                                  graph->axis_offset,
                                                  graph->frame_space,
                                                  graph->grid_offset,
                                                  graph->point_mask_source);
}

/**
 * @brief Set axis offset of graph widget
 *
 * @param[in] graph Graph widget instance
 * @param[in] axis_offset axis offset
 * @return -1 on failure
 */

int tinyui_graph_set_axis_offset(struct tinyui_graph *graph, int axis_offset)
{
    if (graph == 0 || axis_offset < 0) {
        return -1;
    }

    return graph_apply_native_geometry_candidate(graph,
                                                  graph->x_axis,
                                                  graph->y_axis,
                                                  axis_offset,
                                                  graph->frame_space,
                                                  graph->grid_offset,
                                                  graph->point_mask_source);
}

/**
 * @brief Set frame space of graph widget
 *
 * @param[in] graph Graph widget instance
 * @param[in] frame_space frame space
 * @return -1 on failure
 */

int tinyui_graph_set_frame_space(struct tinyui_graph *graph, int frame_space)
{
    if (graph == 0 || frame_space < 0) {
        return -1;
    }

    return graph_apply_native_geometry_candidate(graph,
                                                  graph->x_axis,
                                                  graph->y_axis,
                                                  graph->axis_offset,
                                                  frame_space,
                                                  graph->grid_offset,
                                                  graph->point_mask_source);
}

/**
 * @brief Set grid offset of graph widget
 *
 * @param[in] graph Graph widget instance
 * @param[in] grid_offset grid offset
 * @return -1 on failure
 */

int tinyui_graph_set_grid_offset(struct tinyui_graph *graph, int grid_offset)
{
    if (graph == 0 || grid_offset <= 0) {
        return -1;
    }

    return graph_apply_native_geometry_candidate(graph,
                                                  graph->x_axis,
                                                  graph->y_axis,
                                                  graph->axis_offset,
                                                  graph->frame_space,
                                                  grid_offset,
                                                  graph->point_mask_source);
}

/**
 * @brief Set point mask source of graph widget
 *
 * @param[in] graph Graph widget instance
 * @param[in] source Image source
 * @return -1 on failure
 */

int tinyui_graph_set_point_mask_source(struct tinyui_graph *graph, struct tinyui_image_source *source)
{
    if (graph == 0 || source == 0 || source->mask_tile == 0) {
        return -1;
    }

    return graph_apply_native_geometry_candidate(graph,
                                                  graph->x_axis,
                                                  graph->y_axis,
                                                  graph->axis_offset,
                                                  graph->frame_space,
                                                  graph->grid_offset,
                                                  source);
}

/**
 * @brief Set point image mask of graph widget
 *
 * @param[in] graph Graph widget instance
 * @param[in] source Image source
 * @return 0 on success, -1 on failure
 */

int tinyui_graph_set_point_image_mask(struct tinyui_graph *graph, struct tinyui_image_source *source)
{
    return tinyui_graph_set_point_mask_source(graph, source);
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

int tinyui_graph_add_series(struct tinyui_graph *graph,
                            unsigned int series_color,
                            int line_size,
                            int point_max)
{
    ldGraph_t *ld_graph;
    int series_index;

    if (graph == 0 || line_size < 0 || point_max <= 0 || point_max > TINYUI_GRAPH_MAX_POINTS) {
        return -1;
    }
    if (graph->widget.ld_widget == 0
        || graph->widget.kind != TINYUI_BACKEND_WIDGET_GRAPH) {
        return -1;
    }
    ld_graph = (ldGraph_t *)graph->widget.ld_widget;

    series_index = (int)ldGraphAddSeries(ld_graph,
                                         (ldColor)series_color,
                                         (uint8_t)line_size,
                                         (uint16_t)point_max);
    if (series_index < 0 || series_index >= TINYUI_GRAPH_MAX_SERIES) {
        return -1;
    }

    graph->series_point_counts[series_index] = point_max;
    if (series_index >= graph->series_count) {
        graph->series_count = series_index + 1;
    }
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

int tinyui_graph_set_value(struct tinyui_graph *graph,
                           int series_index,
                           int value_index,
                           int value)
{
    ldGraph_t *ld_graph;

    if (graph == 0 || series_index < 0 || series_index >= graph->series_count ||
        value_index < 0 || value_index >= graph->series_point_counts[series_index] ||
        value < 0) {
        return -1;
    }
    if (graph->widget.ld_widget == 0
        || graph->widget.kind != TINYUI_BACKEND_WIDGET_GRAPH) {
        return -1;
    }
    ld_graph = (ldGraph_t *)graph->widget.ld_widget;
    if (series_index >= ld_graph->seriesCount ||
        value_index >= ld_graph->pSeries[series_index].valueCountMax) {
        return -1;
    }

    ldGraphSetValue(ld_graph, (uint8_t)series_index, (uint16_t)value_index, (uint16_t)value);
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

int tinyui_graph_move_add(struct tinyui_graph *graph, int series_index, int value)
{
    ldGraph_t *ld_graph;

    if (graph == 0 || series_index < 0 || series_index >= graph->series_count || value < 0) {
        return -1;
    }
    if (graph->widget.ld_widget == 0
        || graph->widget.kind != TINYUI_BACKEND_WIDGET_GRAPH) {
        return -1;
    }
    ld_graph = (ldGraph_t *)graph->widget.ld_widget;
    if (series_index >= ld_graph->seriesCount) {
        return -1;
    }

    ldGraphMoveAdd(ld_graph, (uint8_t)series_index, (uint16_t)value);
    return 0;
}

/**
 * @brief Get series count of graph widget
 *
 * @param[in] graph Graph widget instance
 * @return -1 on failure
 */

int tinyui_graph_get_series_count(const struct tinyui_graph *graph)
{
    ldGraph_t *ld_graph;

    if (graph == 0 || graph->widget.ld_widget == 0
        || graph->widget.kind != TINYUI_BACKEND_WIDGET_GRAPH) {
        return -1;
    }
    ld_graph = (ldGraph_t *)graph->widget.ld_widget;

    return (int)ld_graph->seriesCount;
}

/**
 * @brief Get value of graph widget
 *
 * @param[in] graph Graph widget instance
 * @param[in] series_index series index
 * @param[in] value_index value index
 * @return -1 on failure
 */

int tinyui_graph_get_value(const struct tinyui_graph *graph, int series_index, int value_index)
{
    ldGraph_t *ld_graph;

    if (graph == 0 || graph->widget.ld_widget == 0
        || graph->widget.kind != TINYUI_BACKEND_WIDGET_GRAPH) {
        return -1;
    }
    ld_graph = (ldGraph_t *)graph->widget.ld_widget;
    if (series_index < 0 || series_index >= ld_graph->seriesCount ||
        value_index < 0 || value_index >= ld_graph->pSeries[series_index].valueCountMax) {
        return -1;
    }

    return (int)ld_graph->pSeries[series_index].pValueList[value_index];
}
