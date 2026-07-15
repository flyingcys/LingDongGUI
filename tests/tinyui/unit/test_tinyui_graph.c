/*
 * TinyUI graph unit tests — M3 Task 4 L3/L4 harness.
 *
 * Validates series/point capacity, axis/grid/frame/mask round-trip against
 * real ldGraph_t fields, and invalid series/index rejection without polluting
 * other series. Uses the M2 public scaffold (tinyui_init / screen_create).
 */

#include "tinyui.h"
#include "../../../src/gui/ldBase.h"
#include "../../../src/gui/ldGraph.h"
#include "internal.h"
#include "resource/image_source.h"
#include "widgets/graph.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

static arm_2d_tile_t g_large_graph_point_mask = {
    .tRegion = {
        .tSize = {
            .iWidth = 24,
            .iHeight = 24,
        },
    },
};

static arm_2d_tile_t g_default_point_mask = {
    .tRegion = {
        .tSize = {
            .iWidth = 7,
            .iHeight = 7,
        },
    },
};

extern const arm_2d_tile_t c_tile_graphDefalutDot_Mask;

static void bind_test_tiles(tinyui_image_source_t *source,
                            arm_2d_tile_t *img_tile,
                            arm_2d_tile_t *mask_tile)
{
    assert(source != 0);
    memset(source, 0, sizeof(*source));
    source->kind = TINYUI_IMAGE_SOURCE_RGB565_MEMORY;
    if (img_tile != 0) {
        source->width = (uint16_t)img_tile->tRegion.tSize.iWidth;
        source->height = (uint16_t)img_tile->tRegion.tSize.iHeight;
        memcpy(source->_image_private, img_tile, sizeof(*img_tile));
    }
    if (mask_tile != 0) {
        memcpy(source->_mask_private, mask_tile, sizeof(*mask_tile));
    }
}

static struct tinyui_graph *graph_host(tinyui_obj_t *obj)
{
    return (struct tinyui_graph *)(void *)obj;
}

static ldGraph_t *graph_ld(tinyui_obj_t *obj)
{
    struct tinyui_widget *backend = (struct tinyui_widget *)(void *)obj;

    assert(backend != 0);
    assert(backend->ld_widget != 0);
    return (ldGraph_t *)backend->ld_widget;
}

static void test_graph_create_builds_direct_backend_mapping(tinyui_obj_t *root)
{
    tinyui_obj_t *graph = tinyui_graph_create(root);
    struct tinyui_widget *backend;
    struct tinyui_widget *parent_backend;
    struct tinyui_graph *wrapper;
    ldGraph_t *ld_graph;

    assert(graph != 0);
    backend = (struct tinyui_widget *)(void *)graph;
    wrapper = graph_host(graph);
    parent_backend = (struct tinyui_widget *)(void *)root;

    assert(backend->ld_widget != 0);
    assert(parent_backend->ld_widget != 0);
    assert(backend->kind == TINYUI_BACKEND_WIDGET_GRAPH);
    assert(backend->owner == parent_backend->owner);
    assert(ldBaseGetParent((ldBase_t *)backend->ld_widget) ==
           (ldBase_t *)parent_backend->ld_widget);
    assert(backend->ld_name_id != 0);
    assert(backend->ld_event_bridge_scene != 0);
    assert(backend->ld_event_bridge_sender == backend->ld_widget);
    ld_graph = (ldGraph_t *)backend->ld_widget;
    assert(ld_graph != 0);
    assert(wrapper->series_max == 1);
    assert(tinyui_runtime_internal_widget_has_ld_binding(backend) == 1);
}

static void test_graph_native_defaults_are_not_overridden_on_create(tinyui_obj_t *root)
{
    tinyui_obj_t *graph = tinyui_graph_create(root);
    struct tinyui_graph *wrapper;
    ldGraph_t *ld_graph;

    assert(graph != 0);
    wrapper = graph_host(graph);
    ld_graph = graph_ld(graph);

    assert(ld_graph->frameSpace == 10);
    assert(ld_graph->gridOffset == 5);
    assert(ld_graph->ptPointMaskTile == &c_tile_graphDefalutDot_Mask);
    assert(ld_graph->xAxisMax == 220);
    assert(ld_graph->yAxisMax == 220);
    assert(ld_graph->xAxisOffset == 5);

    assert(wrapper->frame_space == 10);
    assert(wrapper->grid_offset == 5);
    assert(wrapper->axis_offset == 5);
    assert(wrapper->x_axis == 220);
    assert(wrapper->y_axis == 100);
    assert(wrapper->point_mask_source == 0);
    assert(wrapper->series_max == 1);
    assert(wrapper->series_count == 0);
}

static void test_graph_series_value_readback_survives_frame_update(tinyui_obj_t *root)
{
    tinyui_graph_props_t props;
    tinyui_obj_t *graph;
    struct tinyui_graph *wrapper;
    ldGraph_t *ld_graph;
    int series;

    memset(&props, 0, sizeof(props));
    props.fields = TINYUI_GRAPH_FIELD_SERIES_MAX;
    props.series_max = 2;

    graph = tinyui_graph_create_with_props(root, &props);
    assert(graph != 0);
    wrapper = graph_host(graph);
    assert(wrapper->series_max == 2);

    series = tinyui_graph_add_series(graph, 0x2057C4U, 2, 4);
    assert(series == 0);
    assert(tinyui_graph_set_value(graph, series, 0, 10) == 0);
    assert(tinyui_graph_set_value(graph, series, 1, 30) == 0);
    assert(tinyui_graph_set_value(graph, series, 2, 55) == 0);
    assert(tinyui_graph_set_value(graph, series, 3, 40) == 0);
    assert(tinyui_graph_get_series_count(graph) == 1);
    assert(tinyui_graph_get_value(graph, series, 2) == 55);

    ld_graph = graph_ld(graph);
    assert(ld_graph->seriesCount == 1);
    assert(ld_graph->pSeries[series].seriesColor ==
           (ldColor)tinyui_rgb_to_ld_color(0x2057C4U));
    assert(ld_graph->pSeries[series].valueCountMax == 4);
    assert(ld_graph->pSeries[series].pValueList[2] == 55);
    assert(wrapper->series_count == 1);
    assert(wrapper->series_point_counts[series] == 4);
}

static void test_graph_visible_output_matches_series_updates(tinyui_obj_t *root)
{
    tinyui_obj_t *graph = tinyui_graph_create(root);
    ldGraph_t *ld_graph;
    int series;

    assert(graph != 0);
    series = tinyui_graph_add_series(graph, 0x418F1FU, 2, 3);
    assert(series == 0);
    assert(tinyui_graph_set_value(graph, series, 0, 5) == 0);
    assert(tinyui_graph_set_value(graph, series, 1, 15) == 0);
    assert(tinyui_graph_set_value(graph, series, 2, 25) == 0);
    assert(tinyui_graph_move_add(graph, series, 45) == 0);
    assert(tinyui_graph_get_value(graph, series, 0) == 15);
    assert(tinyui_graph_get_value(graph, series, 1) == 25);
    assert(tinyui_graph_get_value(graph, series, 2) == 45);

    ld_graph = graph_ld(graph);
    assert(ld_graph->pSeries[series].pValueList[0] == 15);
    assert(ld_graph->pSeries[series].pValueList[1] == 25);
    assert(ld_graph->pSeries[series].pValueList[2] == 45);
}

static void test_graph_capacity_bounds_reject_without_silent_truncation(tinyui_obj_t *root)
{
    tinyui_graph_props_t props;
    tinyui_obj_t *graph;
    struct tinyui_graph *wrapper;
    ldGraph_t *ld_graph;
    int series;
    int i;

    memset(&props, 0, sizeof(props));
    props.fields = TINYUI_GRAPH_FIELD_SERIES_MAX;
    props.series_max = 2;
    graph = tinyui_graph_create_with_props(root, &props);
    assert(graph != 0);
    wrapper = graph_host(graph);
    ld_graph = graph_ld(graph);

    assert(tinyui_graph_add_series(graph, 0x111111U, 1, 0) == -1);
    assert(tinyui_last_result() == TINYUI_ERROR_OUT_OF_RANGE);
    assert(tinyui_graph_add_series(graph, 0x111111U, 1, TINYUI_GRAPH_MAX_POINTS + 1) == -1);
    assert(tinyui_last_result() == TINYUI_ERROR_OUT_OF_RANGE);
    assert(tinyui_graph_add_series(graph, 0x111111U, -1, 4) == -1);
    assert(tinyui_last_result() == TINYUI_ERROR_OUT_OF_RANGE);
    assert(tinyui_graph_get_series_count(graph) == 0);
    assert(ld_graph->seriesCount == 0);

    series = tinyui_graph_add_series(graph, 0x2057C4U, 2, TINYUI_GRAPH_MAX_POINTS);
    assert(series == 0);
    assert(wrapper->series_point_counts[0] == TINYUI_GRAPH_MAX_POINTS);
    assert(ld_graph->pSeries[0].valueCountMax == TINYUI_GRAPH_MAX_POINTS);

    series = tinyui_graph_add_series(graph, 0x41A85FU, 1, 4);
    assert(series == 1);
    assert(tinyui_graph_get_series_count(graph) == 2);
    assert(ld_graph->seriesCount == 2);

    /* series_max=2 is full: no silent growth. */
    assert(tinyui_graph_add_series(graph, 0xFFFFFFU, 1, 4) == -1);
    assert(tinyui_last_result() == TINYUI_ERROR_CAPACITY);
    assert(tinyui_graph_get_series_count(graph) == 2);
    assert(ld_graph->seriesCount == 2);
    assert(wrapper->series_count == 2);

    /* Absolute constant ceiling also fails. */
    memset(&props, 0, sizeof(props));
    props.fields = TINYUI_GRAPH_FIELD_SERIES_MAX;
    props.series_max = TINYUI_GRAPH_MAX_SERIES + 1;
    assert(tinyui_graph_create_with_props(root, &props) == 0);

    props.series_max = 0;
    assert(tinyui_graph_create_with_props(root, &props) == 0);

    props.series_max = TINYUI_GRAPH_MAX_SERIES;
    graph = tinyui_graph_create_with_props(root, &props);
    assert(graph != 0);
    for (i = 0; i < TINYUI_GRAPH_MAX_SERIES; ++i) {
        series = tinyui_graph_add_series(graph, 0x101010U + (unsigned int)i, 1, 2);
        assert(series == i);
    }
    assert(tinyui_graph_add_series(graph, 0xABCDEFU, 1, 2) == -1);
    assert(tinyui_last_result() == TINYUI_ERROR_CAPACITY);
    assert(tinyui_graph_get_series_count(graph) == TINYUI_GRAPH_MAX_SERIES);
}

static void test_graph_create_with_props_maps_fields_and_rejects_invalid(tinyui_obj_t *root)
{
    int user_cookie = 42;
    tinyui_graph_props_t props;
    tinyui_obj_t *graph;
    struct tinyui_graph *wrapper;
    ldGraph_t *ld_graph;
    int first_series;
    int second_series;

    memset(&props, 0, sizeof(props));
    props.fields = TINYUI_GRAPH_FIELD_SERIES_MAX
        | TINYUI_GRAPH_FIELD_STYLE_CLASS
        | TINYUI_GRAPH_FIELD_USER_DATA
        | TINYUI_GRAPH_FIELD_WIDTH
        | TINYUI_GRAPH_FIELD_HEIGHT;
    props.series_max = 3;
    props.style_class = "chart-card";
    props.user_data = &user_cookie;
    props.width = 260;
    props.height = 140;

    graph = tinyui_graph_create_with_props(root, &props);
    assert(graph != 0);
    wrapper = graph_host(graph);
    ld_graph = graph_ld(graph);

    assert(wrapper->series_max == 3);
    assert(wrapper->widget.style_class == (const char *)"chart-card");
    assert(wrapper->widget.user_data == &user_cookie);
    assert(tinyui_runtime_internal_widget_get_width(&wrapper->widget) == 260);
    assert(tinyui_runtime_internal_widget_get_height(&wrapper->widget) == 140);
    assert(ld_graph->use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tSize.iWidth == 260);
    assert(ld_graph->use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tSize.iHeight == 140);

    first_series = tinyui_graph_add_series(graph, 0x2057C4U, 2, 4);
    second_series = tinyui_graph_add_series(graph, 0x41A85FU, 1, 4);
    assert(first_series == 0);
    assert(second_series == 1);
    assert(tinyui_graph_set_value(graph, first_series, 0, 8) == 0);
    assert(tinyui_graph_set_value(graph, first_series, 3, 48) == 0);
    assert(tinyui_graph_set_value(graph, second_series, 0, 5) == 0);
    assert(tinyui_graph_get_series_count(graph) == 2);
    assert(tinyui_graph_get_value(graph, first_series, 3) == 48);
    assert(ld_graph->seriesCount == 2);
    assert(ld_graph->pSeries[first_series].valueCountMax == 4);

    assert(tinyui_graph_set_value(graph, second_series, 4, 99) == -1);
    assert(tinyui_graph_get_value(graph, second_series, 4) == -1);

    memset(&props, 0, sizeof(props));
    props.fields = TINYUI_GRAPH_FIELD_WIDTH;
    props.width = -1;
    assert(tinyui_graph_create_with_props(root, &props) == 0);

    memset(&props, 0, sizeof(props));
    props.fields = TINYUI_GRAPH_FIELD_HEIGHT;
    props.height = -3;
    assert(tinyui_graph_create_with_props(root, &props) == 0);

    assert(tinyui_graph_create(0) == 0);
    assert(tinyui_graph_create_with_props(0, 0) == 0);
}

static void test_graph_native_axis_grid_and_point_mask_round_trip(tinyui_obj_t *root)
{
    tinyui_obj_t *graph = tinyui_graph_create(root);
    struct tinyui_graph *wrapper;
    ldGraph_t *ld_graph;
    tinyui_image_source_t point_source;
    int expected_extent;
    uint16_t expected_axis_offset;

    assert(graph != 0);
    wrapper = graph_host(graph);
    ld_graph = graph_ld(graph);
    bind_test_tiles(&point_source, &g_default_point_mask, &g_default_point_mask);

    assert(tinyui_graph_set_axis(graph, 160, 90) == 0);
    assert(tinyui_graph_set_axis_offset(graph, 7) == 0);
    assert(tinyui_graph_set_frame_space(graph, 14) == 0);
    assert(tinyui_graph_set_grid_offset(graph, 11) == 0);
    assert(tinyui_graph_set_point_mask_source(graph, &point_source) == 0);
    assert(tinyui_graph_set_point_image_mask(graph, &point_source) == 0);

    expected_extent =
        ld_graph->use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tSize.iWidth -
        ld_graph->frameSpace * 2;
    expected_axis_offset = (uint16_t)(7 * ld_graph->xScale);

    assert(wrapper->x_axis == 160);
    assert(wrapper->y_axis == 90);
    assert(wrapper->axis_offset == 7);
    assert(wrapper->frame_space == 14);
    assert(wrapper->grid_offset == 11);
    assert(wrapper->point_mask_source == &point_source);

    assert(ld_graph->xAxisMax == (uint16_t)expected_extent);
    assert(ld_graph->yAxisMax == (uint16_t)expected_extent);
    assert(ld_graph->xAxisOffset == expected_axis_offset);
    assert(ld_graph->frameSpace == 14);
    assert(ld_graph->gridOffset == 11);
    assert(ld_graph->ptPointMaskTile == tinyui_image_source_get_mask_tile(&point_source));
    assert(ld_graph->use_as__ldBase_t.isCorner == true);

    assert(tinyui_graph_set_axis(graph, 0, 90) == -1);
    assert(tinyui_graph_set_axis_offset(graph, -1) == -1);
    assert(tinyui_graph_set_frame_space(graph, -1) == -1);
    assert(tinyui_graph_set_grid_offset(graph, 0) == -1);
    assert(tinyui_graph_set_point_mask_source(graph, 0) == -1);
    assert(tinyui_graph_set_point_image_mask(graph, 0) == -1);

    assert(wrapper->x_axis == 160);
    assert(wrapper->y_axis == 90);
    assert(wrapper->axis_offset == 7);
    assert(wrapper->frame_space == 14);
    assert(wrapper->grid_offset == 11);
    assert(wrapper->point_mask_source == &point_source);
    assert(ld_graph->xAxisMax == (uint16_t)expected_extent);
    assert(ld_graph->yAxisMax == (uint16_t)expected_extent);
    assert(ld_graph->xAxisOffset == expected_axis_offset);
    assert(ld_graph->frameSpace == 14);
    assert(ld_graph->gridOffset == 11);
    assert(ld_graph->ptPointMaskTile == tinyui_image_source_get_mask_tile(&point_source));
    assert(ld_graph->use_as__ldBase_t.isCorner == true);
}

static void test_graph_move_add_and_set_value_reject_invalid_inputs_without_polluting_other_series(
    tinyui_obj_t *root)
{
    tinyui_graph_props_t props;
    tinyui_obj_t *graph;
    ldGraph_t *ld_graph;
    int first_series;
    int second_series;

    memset(&props, 0, sizeof(props));
    props.fields = TINYUI_GRAPH_FIELD_SERIES_MAX;
    props.series_max = 2;
    graph = tinyui_graph_create_with_props(root, &props);
    assert(graph != 0);

    first_series = tinyui_graph_add_series(graph, 0x2057C4U, 2, 3);
    second_series = tinyui_graph_add_series(graph, 0x41A85FU, 1, 3);
    assert(first_series == 0);
    assert(second_series == 1);

    assert(tinyui_graph_set_value(graph, first_series, 0, 11) == 0);
    assert(tinyui_graph_set_value(graph, first_series, 1, 22) == 0);
    assert(tinyui_graph_set_value(graph, first_series, 2, 33) == 0);
    assert(tinyui_graph_set_value(graph, second_series, 0, 7) == 0);
    assert(tinyui_graph_set_value(graph, second_series, 1, 14) == 0);
    assert(tinyui_graph_set_value(graph, second_series, 2, 21) == 0);

    ld_graph = graph_ld(graph);
    assert(tinyui_graph_move_add(graph, first_series, 44) == 0);
    assert(ld_graph->pSeries[first_series].pValueList[0] == 22);
    assert(ld_graph->pSeries[first_series].pValueList[1] == 33);
    assert(ld_graph->pSeries[first_series].pValueList[2] == 44);
    assert(ld_graph->pSeries[second_series].pValueList[0] == 7);
    assert(ld_graph->pSeries[second_series].pValueList[1] == 14);
    assert(ld_graph->pSeries[second_series].pValueList[2] == 21);

    assert(tinyui_graph_set_value(graph, second_series, 3, 99) == -1);
    assert(tinyui_graph_set_value(graph, second_series, 1, -1) == -1);
    assert(tinyui_graph_set_value(graph, -1, 0, 1) == -1);
    assert(tinyui_graph_move_add(graph, second_series, -1) == -1);
    assert(tinyui_graph_move_add(graph, 2, 55) == -1);
    assert(tinyui_graph_move_add(graph, -1, 55) == -1);

    assert(ld_graph->pSeries[first_series].pValueList[0] == 22);
    assert(ld_graph->pSeries[first_series].pValueList[1] == 33);
    assert(ld_graph->pSeries[first_series].pValueList[2] == 44);
    assert(ld_graph->pSeries[second_series].pValueList[0] == 7);
    assert(ld_graph->pSeries[second_series].pValueList[1] == 14);
    assert(ld_graph->pSeries[second_series].pValueList[2] == 21);
}

static void test_graph_init_and_shared_base_aliases_round_trip(tinyui_obj_t *root)
{
    tinyui_obj_t *graph = tinyui_graph_create(root);
    struct tinyui_widget *backend;
    ldBase_t *ld_base;

    assert(graph != 0);
    backend = (struct tinyui_widget *)(void *)graph;
    ld_base = (ldBase_t *)backend->ld_widget;
    assert(ld_base != 0);

    assert(tinyui_obj_set_pos(graph, 17, 29) == TINYUI_OK);
    assert(ld_base->use_as__arm_2d_control_node_t.tRegion.tLocation.iX == 17);
    assert(ld_base->use_as__arm_2d_control_node_t.tRegion.tLocation.iY == 29);
    assert(tinyui_obj_set_visible(graph, 0) == TINYUI_OK);
    assert(tinyui_obj_set_opacity(graph, 81) == TINYUI_OK);
    assert(tinyui_obj_set_selectable(graph, 1) == TINYUI_OK);
    assert(tinyui_obj_set_selected(graph, 1) == TINYUI_OK);
    assert(tinyui_runtime_internal_widget_set_corner(backend, 1) == 0);

    assert(ld_base->isHidden == true);
    assert(ld_base->opacity == 81);
    assert(ld_base->isSelectable == true);
    assert(ld_base->isSelected == true);
    assert(ld_base->isCorner == true);
}

static void test_graph_rejects_non_graph_backend_binding(tinyui_obj_t *root)
{
    tinyui_obj_t *graph = tinyui_graph_create(root);
    struct tinyui_widget *backend;
    struct tinyui_graph *wrapper;
    ldGraph_t *ld_graph;
    tinyui_image_source_t large_point_source;
    int original_kind;
    int original_x_axis;
    int original_y_axis;
    int original_axis_offset_host;
    int original_frame_space_host;
    int original_grid_offset_host;
    uint16_t original_axis_offset;
    uint8_t original_frame_space_native;
    uint8_t original_grid_offset_native;

    assert(graph != 0);
    wrapper = graph_host(graph);
    backend = (struct tinyui_widget *)(void *)graph;
    ld_graph = graph_ld(graph);
    bind_test_tiles(&large_point_source, &g_large_graph_point_mask, &g_large_graph_point_mask);

    original_kind = backend->kind;
    original_x_axis = wrapper->x_axis;
    original_y_axis = wrapper->y_axis;
    original_axis_offset_host = wrapper->axis_offset;
    original_frame_space_host = wrapper->frame_space;
    original_grid_offset_host = wrapper->grid_offset;
    original_axis_offset = ld_graph->xAxisOffset;
    original_frame_space_native = ld_graph->frameSpace;
    original_grid_offset_native = ld_graph->gridOffset;
    backend->kind = TINYUI_BACKEND_WIDGET_BUTTON;

    assert(tinyui_graph_set_axis(graph, 180, 120) == -1);
    assert(wrapper->x_axis == original_x_axis);
    assert(wrapper->y_axis == original_y_axis);
    assert(tinyui_graph_set_axis_offset(graph, 17) == -1);
    assert(wrapper->axis_offset == original_axis_offset_host);
    assert(tinyui_graph_set_frame_space(graph, 19) == -1);
    assert(wrapper->frame_space == original_frame_space_host);
    assert(tinyui_graph_set_grid_offset(graph, 13) == -1);
    assert(wrapper->grid_offset == original_grid_offset_host);
    assert(tinyui_graph_set_point_mask_source(graph, &large_point_source) == -1);
    assert(wrapper->point_mask_source == 0);
    assert(tinyui_graph_add_series(graph, 0x2057C4U, 2, 4) == -1);
    assert(tinyui_graph_get_series_count(graph) == -1);
    assert(tinyui_graph_get_value(graph, 0, 0) == -1);

    assert(ld_graph->seriesCount == 0);
    assert(ld_graph->xAxisOffset == original_axis_offset);
    assert(ld_graph->frameSpace == original_frame_space_native);
    assert(ld_graph->gridOffset == original_grid_offset_native);

    backend->kind = original_kind;
}

static void test_graph_point_mask_larger_than_frame_space_is_accepted_and_synced(tinyui_obj_t *root)
{
    tinyui_obj_t *graph = tinyui_graph_create(root);
    struct tinyui_graph *wrapper;
    ldGraph_t *ld_graph;
    tinyui_image_source_t large_point_source;
    int original_frame_space_host;

    assert(graph != 0);
    wrapper = graph_host(graph);
    ld_graph = graph_ld(graph);
    bind_test_tiles(&large_point_source, &g_large_graph_point_mask, &g_large_graph_point_mask);

    original_frame_space_host = wrapper->frame_space;
    assert(original_frame_space_host < g_large_graph_point_mask.tRegion.tSize.iWidth);
    assert(tinyui_graph_set_point_mask_source(graph, &large_point_source) == 0);

    assert(wrapper->point_mask_source == &large_point_source);
    assert(wrapper->frame_space == g_large_graph_point_mask.tRegion.tSize.iWidth);
    assert(ld_graph->ptPointMaskTile == tinyui_image_source_get_mask_tile(&large_point_source));
    assert(ld_graph->frameSpace == g_large_graph_point_mask.tRegion.tSize.iWidth);
}

static void test_graph_rejects_null_args(void)
{
    assert(tinyui_graph_create(0) == 0);
    assert(tinyui_graph_add_series(0, 0xFFFFFFU, 1, 10) == -1);
    assert(tinyui_graph_set_value(0, 0, 0, 50) == -1);
    assert(tinyui_graph_get_value(0, 0, 0) == -1);
    assert(tinyui_graph_move_add(0, 0, 1) == -1);
    assert(tinyui_graph_get_series_count(0) == -1);
    assert(tinyui_graph_set_axis(0, 100, 100) == -1);
    assert(tinyui_graph_set_axis_offset(0, 1) == -1);
    assert(tinyui_graph_set_frame_space(0, 1) == -1);
    assert(tinyui_graph_set_grid_offset(0, 1) == -1);
    assert(tinyui_graph_set_point_mask_source(0, 0) == -1);
    assert(tinyui_graph_set_point_image_mask(0, 0) == -1);
}

int main(void)
{
    tinyui_obj_t *root;

    tinyui_deinit();
    assert(tinyui_init() == TINYUI_OK);
    root = tinyui_screen_create();
    assert(root != 0);

    test_graph_create_builds_direct_backend_mapping(root);
    test_graph_native_defaults_are_not_overridden_on_create(root);
    test_graph_series_value_readback_survives_frame_update(root);
    test_graph_visible_output_matches_series_updates(root);
    test_graph_capacity_bounds_reject_without_silent_truncation(root);
    test_graph_create_with_props_maps_fields_and_rejects_invalid(root);
    test_graph_native_axis_grid_and_point_mask_round_trip(root);
    test_graph_move_add_and_set_value_reject_invalid_inputs_without_polluting_other_series(root);
    test_graph_init_and_shared_base_aliases_round_trip(root);
    test_graph_rejects_non_graph_backend_binding(root);
    test_graph_point_mask_larger_than_frame_space_is_accepted_and_synced(root);
    test_graph_rejects_null_args();

    tinyui_deinit();
    return 0;
}
