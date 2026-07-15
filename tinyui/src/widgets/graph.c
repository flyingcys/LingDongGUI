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
#include "widgets/graph.h"
#include "../core/runtime_bridge.h"
#include "../../../src/gui/ldGraph.h"

#include <stdlib.h>


static struct tinyui_graph *tinyui_graph_as_graph(tinyui_obj_t *obj)
{
    struct tinyui_widget *w = (struct tinyui_widget *)(void *)obj;
    if (w == 0 || !tinyui_runtime_internal_widget_is_kind(w, TINYUI_BACKEND_WIDGET_GRAPH)) {
        return 0;
    }
    return (struct tinyui_graph *)w;
}

static const struct tinyui_graph *tinyui_graph_as_graph_const(const tinyui_obj_t *obj)
{
    const struct tinyui_widget *w = (const struct tinyui_widget *)(const void *)obj;
    if (w == 0 || !tinyui_runtime_internal_widget_is_kind(w, TINYUI_BACKEND_WIDGET_GRAPH)) {
        return 0;
    }
    return (const struct tinyui_graph *)w;
}

/* ---- C2 depose closure state (file-static, single-threaded scope) ---- */


struct tinyui_graph_create_ctx {
    int series_max;
};

static int graph_host_axis_from_native(const ldGraph_t *ld_graph, float scale)
{
    int extent;

    if (ld_graph == 0 || scale <= 0.0f) {
        return 0;
    }

    extent = ld_graph->use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tSize.iWidth -
             ld_graph->frameSpace * 2;
    if (extent <= 0) {
        return 0;
    }

    return (int)((float)extent / scale + 0.5f);
}

static void graph_sync_host_geometry_from_ld(struct tinyui_graph *graph)
{
    ldGraph_t *ld_graph;

    if (graph == 0 || graph->widget.ld_widget == 0) {
        return;
    }

    ld_graph = (ldGraph_t *)graph->widget.ld_widget;
    graph->x_axis = graph_host_axis_from_native(ld_graph, ld_graph->xScale);
    graph->y_axis = graph_host_axis_from_native(ld_graph, ld_graph->yScale);
    if (ld_graph->xScale > 0.0f) {
        graph->axis_offset = (int)((float)ld_graph->xAxisOffset / ld_graph->xScale + 0.5f);
    } else {
        graph->axis_offset = (int)ld_graph->xAxisOffset;
    }
    graph->frame_space = (int)ld_graph->frameSpace;
    graph->grid_offset = (int)ld_graph->gridOffset;
}

static void *tinyui_runtime_internal_graph_ld_init(void *ctx,
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
    if (point_mask_source != 0 && tinyui_image_source_get_mask_tile(point_mask_source) != 0) {
        point_mask_tile = (arm_2d_tile_t *)tinyui_image_source_get_mask_tile(point_mask_source);
        point_mask_width = point_mask_tile->tRegion.tSize.iWidth;
        if (effective_frame_space < point_mask_width) {
            effective_frame_space = point_mask_width;
        }
    } else {
        point_mask_tile = 0;
    }

    ldGraphSetFrameSpace(ld_graph,
                         (uint8_t)effective_frame_space,
                         ld_graph->use_as__ldBase_t.isCorner);
    ldGraphSetAxis(ld_graph, (uint16_t)x_axis, (uint16_t)y_axis, (uint16_t)axis_offset);
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
    if (ld_graph->xAxisOffset != (uint16_t)(axis_offset * ld_graph->xScale)) {
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

tinyui_obj_t *tinyui_graph_create(tinyui_obj_t *parent)
{
    struct tinyui_widget *parent_w = (struct tinyui_widget *)(void *)parent;
    const char *id = "graph";
    if (parent_w == 0) { return 0; }
    int series_max = 1;

    struct tinyui_graph *graph;
    struct tinyui_graph_create_ctx create_ctx;
    ldGraph_t *ld_graph;

    if (parent_w == 0 || id == 0 || series_max <= 0 || series_max > TINYUI_GRAPH_MAX_SERIES) {
        return 0;
    }

    if (parent_w->owner == 0
        || parent_w->owner->ld_scene == 0
        || parent_w->ld_widget == 0) {
        return 0;
    }

    create_ctx.series_max = series_max;
    graph = (struct tinyui_graph *)tinyui_runtime_internal_widget_create_leaf(parent_w,
                                                             TINYUI_BACKEND_WIDGET_GRAPH,
                                                             tinyui_runtime_internal_graph_ld_init,
                                                             &create_ctx,
                                                             sizeof(*graph));
    if (graph == 0) {
        return 0;
    }

    graph->id = id;
    graph->series_max = series_max;
    ld_graph = (ldGraph_t *)graph->widget.ld_widget;
    if (ld_graph == 0) {
        tinyui_runtime_internal_widget_destroy_common(&graph->widget);
        return 0;
    }
    graph_sync_host_geometry_from_ld(graph);
    graph->point_mask_source = 0;
    graph->widget.visible = 1;
    graph->widget.enabled = 1;
    return (tinyui_obj_t *)graph;
}

/**
 * @brief Create graph widget with properties
 *
 * @param[in] parent Parent widget
 * @param[in] props Properties structure
 * @return Pointer to the object on success, NULL on failure
 */

tinyui_obj_t *tinyui_graph_create_with_props(tinyui_obj_t *parent,
                                             const tinyui_graph_props_t *props)
{
    tinyui_obj_t *obj;
    struct tinyui_graph *graph;
    struct tinyui_widget *parent_w;
    const char *id = "graph";
    int series_max = 1;
    struct tinyui_graph_create_ctx create_ctx;

    if (props == 0) {
        return tinyui_graph_create(parent);
    }

    parent_w = (struct tinyui_widget *)(void *)parent;
    if (parent_w == 0) {
        return 0;
    }
    if ((props->fields & TINYUI_GRAPH_FIELD_SERIES_MAX) != 0) {
        series_max = props->series_max;
    }
    if (series_max <= 0 || series_max > TINYUI_GRAPH_MAX_SERIES) {
        return 0;
    }
    if ((props->fields & TINYUI_GRAPH_FIELD_WIDTH) != 0 && props->width < 0) {
        return 0;
    }
    if ((props->fields & TINYUI_GRAPH_FIELD_HEIGHT) != 0 && props->height < 0) {
        return 0;
    }
    if (parent_w->ld_widget == 0 || parent_w->owner == 0) {
        return 0;
    }

    create_ctx.series_max = series_max;
    graph = (struct tinyui_graph *)tinyui_runtime_internal_widget_create_leaf(parent_w,
                                                             TINYUI_BACKEND_WIDGET_GRAPH,
                                                             tinyui_runtime_internal_graph_ld_init,
                                                             &create_ctx,
                                                             sizeof(*graph));
    if (graph == 0) {
        return 0;
    }
    graph->id = id;
    graph->series_max = series_max;
    graph->point_mask_source = 0;
    graph_sync_host_geometry_from_ld(graph);
    graph->widget.visible = 1;
    graph->widget.enabled = 1;
    obj = (tinyui_obj_t *)graph;

    if ((props->fields & TINYUI_GRAPH_FIELD_ID) != 0) {
        (void)props->id;
    }
    if ((props->fields & TINYUI_GRAPH_FIELD_USER_DATA) != 0) {
        if (tinyui_runtime_internal_widget_set_user_data(&graph->widget, props->user_data) != 0) {
            (void)tinyui_obj_delete(obj);
            return 0;
        }
    }
    if ((props->fields & TINYUI_GRAPH_FIELD_STYLE_CLASS) != 0) {
        if (tinyui_runtime_internal_widget_set_style_class(&graph->widget, props->style_class) != 0) {
            (void)tinyui_obj_delete(obj);
            return 0;
        }
    }
    if ((props->fields & TINYUI_GRAPH_FIELD_WIDTH) != 0 || (props->fields & TINYUI_GRAPH_FIELD_HEIGHT) != 0) {
        int w = tinyui_runtime_internal_widget_get_width(&graph->widget);
        int h = tinyui_runtime_internal_widget_get_height(&graph->widget);
        if (w < 0) { w = 0; }
        if (h < 0) { h = 0; }
        if ((props->fields & TINYUI_GRAPH_FIELD_WIDTH) != 0) { w = props->width; }
        if ((props->fields & TINYUI_GRAPH_FIELD_HEIGHT) != 0) { h = props->height; }
        if (tinyui_runtime_internal_widget_set_size(&graph->widget, w, h) != 0) {
            (void)tinyui_obj_delete(obj);
            return 0;
        }
        /* Size changes affect axis scale; keep host mirrors aligned with ld. */
        graph_sync_host_geometry_from_ld(graph);
    }
    return obj;
}



/**
 * @brief Set axis of graph widget
 *
 * @param[in] graph Graph widget instance
 * @param[in] x_axis x axis
 * @param[in] y_axis y axis
 * @return -1 on failure
 */

int tinyui_graph_set_axis(tinyui_obj_t *graph_obj, int x_axis, int y_axis)
{
    struct tinyui_graph *graph = tinyui_graph_as_graph(graph_obj);
    if (graph == 0) { return -1; }

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

int tinyui_graph_set_axis_offset(tinyui_obj_t *graph_obj, int axis_offset)
{
    struct tinyui_graph *graph = tinyui_graph_as_graph(graph_obj);
    if (graph == 0) { return -1; }

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

int tinyui_graph_set_frame_space(tinyui_obj_t *graph_obj, int frame_space)
{
    struct tinyui_graph *graph = tinyui_graph_as_graph(graph_obj);
    if (graph == 0) { return -1; }

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

int tinyui_graph_set_grid_offset(tinyui_obj_t *graph_obj, int grid_offset)
{
    struct tinyui_graph *graph = tinyui_graph_as_graph(graph_obj);
    if (graph == 0) { return -1; }

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

int tinyui_graph_set_point_mask_source(tinyui_obj_t *graph_obj, struct tinyui_image_source *source)
{
    struct tinyui_graph *graph = tinyui_graph_as_graph(graph_obj);
    if (graph == 0) { return -1; }

    if (graph == 0 || source == 0 || tinyui_image_source_get_mask_tile(source) == 0) {
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

int tinyui_graph_set_point_image_mask(tinyui_obj_t *graph_obj, struct tinyui_image_source *source)
{
    struct tinyui_graph *graph = tinyui_graph_as_graph(graph_obj);
    if (graph == 0) { return -1; }

    return tinyui_graph_set_point_mask_source((tinyui_obj_t *)graph, source);
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

int tinyui_graph_add_series(tinyui_obj_t *graph_obj, unsigned int series_color, int line_size, int point_max)
{
    struct tinyui_graph *graph = tinyui_graph_as_graph(graph_obj);
    if (graph == 0) { return -1; }

    ldGraph_t *ld_graph;
    int series_index;

    if (graph == 0) {
        return -1;
    }
    if (line_size < 0 || point_max <= 0 || point_max > TINYUI_GRAPH_MAX_POINTS) {
        tinyui_runtime_set_last_result(TINYUI_ERROR_OUT_OF_RANGE);
        return -1;
    }
    if (graph->series_count >= graph->series_max
        || graph->series_count >= TINYUI_GRAPH_MAX_SERIES) {
        tinyui_runtime_set_last_result(TINYUI_ERROR_CAPACITY);
        return -1;
    }
    if (graph->widget.ld_widget == 0
        || graph->widget.kind != TINYUI_BACKEND_WIDGET_GRAPH) {
        return -1;
    }
    ld_graph = (ldGraph_t *)graph->widget.ld_widget;
    if (ld_graph->seriesCount >= ld_graph->seriesMax
        || ld_graph->seriesCount >= TINYUI_GRAPH_MAX_SERIES) {
        tinyui_runtime_set_last_result(TINYUI_ERROR_CAPACITY);
        return -1;
    }

    series_index = (int)ldGraphAddSeries(ld_graph,
                                         (ldColor)tinyui_rgb_to_ld_color(series_color),
                                         (uint8_t)line_size,
                                         (uint16_t)point_max);
    if (series_index < 0
        || series_index >= graph->series_max
        || series_index >= TINYUI_GRAPH_MAX_SERIES) {
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

int tinyui_graph_set_value(tinyui_obj_t *graph_obj, int series_index, int value_index, int value)
{
    struct tinyui_graph *graph = tinyui_graph_as_graph(graph_obj);
    if (graph == 0) { return -1; }

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

int tinyui_graph_move_add(tinyui_obj_t *graph_obj, int series_index, int value)
{
    struct tinyui_graph *graph = tinyui_graph_as_graph(graph_obj);
    if (graph == 0) { return -1; }

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

int tinyui_graph_get_series_count(const tinyui_obj_t *graph_obj)
{
    const struct tinyui_graph *graph = tinyui_graph_as_graph_const(graph_obj);
    if (graph == 0) { return -1; }

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

int tinyui_graph_get_value(const tinyui_obj_t *graph_obj, int series_index, int value_index)
{
    const struct tinyui_graph *graph = tinyui_graph_as_graph_const(graph_obj);
    if (graph == 0) { return -1; }

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
