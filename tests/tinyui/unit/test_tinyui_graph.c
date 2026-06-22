#include "app.h"
#include "graph.h"
#include "window.h"
#include "../../../src/gui/ldGraph.h"
#include "../../../src/gui/ldBase.h"
#include "internal.h"
#include "tinyui_test_support.h"

#include <assert.h>
#include <string.h>

extern int tinyui_widget_has_ld_binding(const struct tinyui_widget *widget);
static void assert_source_lacks_function_definition(const char *path, const char *symbol)
{
    char needle[256];

    snprintf(needle, sizeof(needle), "static int %s(", symbol);
    if (strstr(symbol, "get_ld_const") != 0) {
        snprintf(needle, sizeof(needle), "static const ldGraph_t *%s(", symbol);
    } else if (strstr(symbol, "get_ld") != 0) {
        snprintf(needle, sizeof(needle), "static ldGraph_t *%s(", symbol);
    }
    assert(tinyui_test_source_contains(path, needle) == 0);
}

static void assert_source_has_function_definition(const char *path,
                                                  const char *prefix,
                                                  const char *symbol)
{
    char needle[256];

    snprintf(needle, sizeof(needle), "%s%s(", prefix, symbol);
    assert(tinyui_test_source_contains(path, needle) == 1);
}

static arm_2d_tile_t g_large_graph_point_mask = {
    .tRegion = {
        .tSize = {
            .iWidth = 24,
            .iHeight = 24,
        },
    },
};

static void test_graph_create_builds_direct_backend_mapping(struct tinyui_window *win)
{
    struct tinyui_graph *graph = tinyui_graph_create(win, "graph_direct_mapping", 2);
    struct tinyui_widget *backend;
    struct tinyui_widget *parent_backend;
    ldGraph_t *ld_graph;

    assert(graph != 0);
    backend = &graph->widget;
    assert(backend->ld_widget != 0);
    parent_backend = &win->widget;
    assert(parent_backend->ld_widget != 0);
    assert(backend->kind == TINYUI_BACKEND_WIDGET_GRAPH);
    assert(backend->owner == parent_backend->owner);
    assert((ldBase_t *)ldBaseGetRootNode((arm_2d_control_node_t *)backend->ld_widget) == (ldBase_t *)ldBaseGetRootNode((arm_2d_control_node_t *)parent_backend->ld_widget));
    assert(ldBaseGetParent((ldBase_t *)backend->ld_widget) == (ldBase_t *)parent_backend->ld_widget);
    assert(backend->ld_name_id != 0);
    assert(backend->ld_event_bridge_scene != 0);
    assert(backend->ld_event_bridge_sender == backend->ld_widget);
    ld_graph = (ldGraph_t *)backend->ld_widget;
    assert(ld_graph != 0);
    assert(((ldBase_t *)ld_graph)->pInfo == backend);
    assert(tinyui_widget_has_ld_binding(&graph->widget) == 1);
}

static void test_graph_series_value_readback_survives_frame_update(void)
{
    struct tinyui_app *app;
    struct tinyui_window *win;
    struct tinyui_graph *graph;
    struct tinyui_widget *backend;
    ldGraph_t *ld_graph;
    int series;

    app = tinyui_app_create();
    assert(app != 0);
    win = tinyui_window_create(app, "root");
    assert(win != 0);
    graph = tinyui_graph_create(win, "graph_readback", 2);
    assert(graph != 0);

    series = tinyui_graph_add_series(graph, 0x2057C4U, 2, 4);
    assert(series == 0);
    assert(tinyui_graph_set_value(graph, series, 0, 10) == 0);
    assert(tinyui_graph_set_value(graph, series, 1, 30) == 0);
    assert(tinyui_graph_set_value(graph, series, 2, 55) == 0);
    assert(tinyui_graph_set_value(graph, series, 3, 40) == 0);
    assert(tinyui_graph_get_series_count(graph) == 1);
    assert(tinyui_graph_get_value(graph, series, 2) == 55);

    backend = &graph->widget;
    assert(backend->ld_widget != 0);
    ld_graph = (ldGraph_t *)backend->ld_widget;
    assert(ld_graph != 0);
    assert(ld_graph->seriesCount == 1);
    assert(ld_graph->pSeries[series].pValueList[2] == 55);
    tinyui_app_destroy(app);
}

static void test_graph_visible_output_matches_series_updates(void)
{
    struct tinyui_app *app;
    struct tinyui_window *win;
    struct tinyui_graph *graph;
    struct tinyui_widget *backend;
    ldGraph_t *ld_graph;
    int series;

    app = tinyui_app_create();
    assert(app != 0);
    win = tinyui_window_create(app, "root");
    assert(win != 0);
    graph = tinyui_graph_create(win, "graph_move_add", 1);
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

    backend = &graph->widget;
    assert(backend->ld_widget != 0);
    ld_graph = (ldGraph_t *)backend->ld_widget;
    assert(ld_graph != 0);
    assert(ld_graph->pSeries[series].pValueList[0] == 15);
    assert(ld_graph->pSeries[series].pValueList[1] == 25);
    assert(ld_graph->pSeries[series].pValueList[2] == 45);
    tinyui_app_destroy(app);
}

static void test_graph_final_release_contract_covers_advanced_readback_boundary(void)
{
    struct tinyui_app *app;
    struct tinyui_window *win;
    struct tinyui_graph *graph;
    struct tinyui_widget *backend;
    ldGraph_t *ld_graph;
    int first_series;
    int second_series;

    app = tinyui_app_create();
    assert(app != 0);
    win = tinyui_window_create(app, "graph_release_root");
    assert(win != 0);
    graph = tinyui_graph_create_with_props(
        win,
        &(struct tinyui_graph_props){
            .id = "graph_release_ready",
            .series_max = 3,
            .style_class = "chart-card",
            .width = 260,
            .height = 140,
        });
    assert(graph != 0);

    first_series = tinyui_graph_add_series(graph, 0x2057C4U, 2, 4);
    second_series = tinyui_graph_add_series(graph, 0x41A85FU, 1, 4);
    assert(first_series == 0);
    assert(second_series == 1);
    assert(tinyui_graph_set_value(graph, first_series, 0, 8) == 0);
    assert(tinyui_graph_set_value(graph, first_series, 1, 16) == 0);
    assert(tinyui_graph_set_value(graph, first_series, 2, 32) == 0);
    assert(tinyui_graph_set_value(graph, first_series, 3, 48) == 0);
    assert(tinyui_graph_set_value(graph, second_series, 0, 5) == 0);
    assert(tinyui_graph_set_value(graph, second_series, 1, 10) == 0);
    assert(tinyui_graph_set_value(graph, second_series, 2, 15) == 0);
    assert(tinyui_graph_set_value(graph, second_series, 3, 20) == 0);

    backend = &graph->widget;
    assert(backend->ld_widget != 0);
    assert(backend->kind == TINYUI_BACKEND_WIDGET_GRAPH);
    assert(backend->style_class == (const char *)"chart-card");
    ld_graph = (ldGraph_t *)backend->ld_widget;
    assert(ld_graph != 0);

    assert(tinyui_graph_get_series_count(graph) == 2);
    assert(tinyui_graph_get_value(graph, first_series, 3) == 48);
    assert(tinyui_graph_get_value(graph, second_series, 0) == 5);
    assert(ld_graph->seriesCount == 2);
    assert(ld_graph->pSeries[first_series].valueCountMax == 4);
    assert(ld_graph->pSeries[second_series].valueCountMax == 4);
    assert(tinyui_graph_add_series(graph, 0xFFFFFFU, -1, 4) == -1);
    assert(tinyui_graph_set_value(graph, second_series, 4, 99) == -1);
    assert(tinyui_graph_get_value(graph, second_series, 4) == -1);

    tinyui_app_destroy(app);
}

static void test_graph_native_axis_grid_and_point_mask_round_trip(void)
{
    struct tinyui_app *app;
    struct tinyui_window *win;
    struct tinyui_graph *graph;
    struct tinyui_widget *backend;
    ldGraph_t *ld_graph;
    struct tinyui_image_source point_source = {
        .img_tile = (arm_2d_tile_t *)&c_tileWhiteDotMask,
        .mask_tile = (arm_2d_tile_t *)&c_tileWhiteDotMask,
    };
    int expected_extent;

    app = tinyui_app_create();
    assert(app != 0);
    win = tinyui_window_create(app, "graph_native_root");
    assert(win != 0);
    graph = tinyui_graph_create(win, "graph_native_round_trip", 2);
    assert(graph != 0);

    assert(tinyui_graph_set_axis(graph, 160, 90) == 0);
    assert(tinyui_graph_set_axis_offset(graph, 7) == 0);
    assert(tinyui_graph_set_frame_space(graph, 14) == 0);
    assert(tinyui_graph_set_grid_offset(graph, 11) == 0);
    assert(tinyui_graph_set_point_mask_source(graph, &point_source) == 0);

    backend = &graph->widget;
    assert(backend->ld_widget != 0);
    ld_graph = (ldGraph_t *)backend->ld_widget;
    assert(ld_graph != 0);

    expected_extent =
        ld_graph->use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tSize.iWidth -
        ld_graph->frameSpace * 2;
    assert(ld_graph->xAxisMax == (uint16_t)expected_extent);
    assert(ld_graph->yAxisMax == (uint16_t)expected_extent);
    assert(ld_graph->xAxisOffset == 7);
    assert(ld_graph->frameSpace == 14);
    assert(ld_graph->gridOffset == 11);
    assert(ld_graph->ptPointMaskTile == point_source.mask_tile);

    assert(tinyui_graph_set_axis(graph, 0, 90) == -1);
    assert(tinyui_graph_set_axis_offset(graph, -1) == -1);
    assert(tinyui_graph_set_frame_space(graph, -1) == -1);
    assert(tinyui_graph_set_grid_offset(graph, 0) == -1);
    assert(tinyui_graph_set_point_mask_source(graph, 0) == -1);

    assert(ld_graph->xAxisMax == (uint16_t)expected_extent);
    assert(ld_graph->yAxisMax == (uint16_t)expected_extent);
    assert(ld_graph->xAxisOffset == 7);
    assert(ld_graph->frameSpace == 14);
    assert(ld_graph->gridOffset == 11);
    assert(ld_graph->ptPointMaskTile == point_source.mask_tile);

    tinyui_app_destroy(app);
}

static void test_graph_move_add_and_set_value_reject_invalid_inputs_without_polluting_other_series(void)
{
    struct tinyui_app *app;
    struct tinyui_window *win;
    struct tinyui_graph *graph;
    struct tinyui_widget *backend;
    ldGraph_t *ld_graph;
    int first_series;
    int second_series;

    app = tinyui_app_create();
    assert(app != 0);
    win = tinyui_window_create(app, "graph_series_guard_root");
    assert(win != 0);
    graph = tinyui_graph_create(win, "graph_series_guard", 2);
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

    backend = &graph->widget;
    assert(backend->ld_widget != 0);
    ld_graph = (ldGraph_t *)backend->ld_widget;
    assert(ld_graph != 0);

    assert(tinyui_graph_move_add(graph, first_series, 44) == 0);
    assert(ld_graph->pSeries[first_series].pValueList[0] == 22);
    assert(ld_graph->pSeries[first_series].pValueList[1] == 33);
    assert(ld_graph->pSeries[first_series].pValueList[2] == 44);
    assert(ld_graph->pSeries[second_series].pValueList[0] == 7);
    assert(ld_graph->pSeries[second_series].pValueList[1] == 14);
    assert(ld_graph->pSeries[second_series].pValueList[2] == 21);

    assert(tinyui_graph_set_value(graph, second_series, 3, 99) == -1);
    assert(tinyui_graph_set_value(graph, second_series, 1, -1) == -1);
    assert(tinyui_graph_move_add(graph, second_series, -1) == -1);
    assert(tinyui_graph_move_add(graph, 2, 55) == -1);

    assert(ld_graph->pSeries[first_series].pValueList[0] == 22);
    assert(ld_graph->pSeries[first_series].pValueList[1] == 33);
    assert(ld_graph->pSeries[first_series].pValueList[2] == 44);
    assert(ld_graph->pSeries[second_series].pValueList[0] == 7);
    assert(ld_graph->pSeries[second_series].pValueList[1] == 14);
    assert(ld_graph->pSeries[second_series].pValueList[2] == 21);

    tinyui_app_destroy(app);
}

static void test_graph_init_and_shared_base_aliases_round_trip(void)
{
    struct tinyui_app *app;
    struct tinyui_window *win;
    struct tinyui_graph *graph;
    struct tinyui_widget *backend;
    ldBase_t *ld_base;

    app = tinyui_app_create();
    assert(app != 0);
    win = tinyui_window_create(app, "graph_base_root");
    assert(win != 0);
    graph = tinyui_graph_create(win, "graph_base_aliases", 2);
    assert(graph != 0);
    backend = &graph->widget;
    assert(backend->ld_widget != 0);
    ld_base = (ldBase_t *)backend->ld_widget;
    assert(ld_base != 0);

    assert(tinyui_widget_set_pos(&graph->widget, 17, 29) == 0);
    assert(ld_base->use_as__arm_2d_control_node_t.tRegion.tLocation.iX == 17);
    assert(ld_base->use_as__arm_2d_control_node_t.tRegion.tLocation.iY == 29);
    assert(tinyui_widget_set_visible(&graph->widget, 0) == 0);
    assert(tinyui_widget_set_opacity(&graph->widget, 81) == 0);
    assert(tinyui_widget_set_selectable(&graph->widget, 1) == 0);
    assert(tinyui_widget_set_selected(&graph->widget, 1) == 0);
    assert(tinyui_widget_set_corner(&graph->widget, 1) == 0);

    assert(ld_base->isHidden == true);
    assert(ld_base->opacity == 81);
    assert(ld_base->isSelectable == true);
    assert(ld_base->isSelected == true);
    assert(ld_base->isCorner == true);
    tinyui_app_destroy(app);
}

static void test_graph_rejects_non_graph_backend_binding(void)
{
    struct tinyui_app *app;
    struct tinyui_window *win;
    struct tinyui_graph *graph;
    struct tinyui_widget *backend;
    ldGraph_t *ld_graph;
    int original_kind;
    int original_x_axis;
    int original_y_axis;
    int original_axis_offset_host;
    int original_frame_space_host;
    int original_grid_offset_host;
    uint16_t original_axis_offset;
    uint8_t original_frame_space_native;
    uint8_t original_grid_offset_native;

    app = tinyui_app_create();
    assert(app != 0);
    win = tinyui_window_create(app, "graph_binding_guard_root");
    assert(win != 0);
    graph = tinyui_graph_create(win, "graph_binding_guard", 2);
    assert(graph != 0);

    backend = &graph->widget;
    assert(backend->ld_widget != 0);
    ld_graph = (ldGraph_t *)backend->ld_widget;
    assert(ld_graph != 0);

    original_kind = backend->kind;
    original_x_axis = graph->x_axis;
    original_y_axis = graph->y_axis;
    original_axis_offset_host = graph->axis_offset;
    original_frame_space_host = graph->frame_space;
    original_grid_offset_host = graph->grid_offset;
    original_axis_offset = ld_graph->xAxisOffset;
    original_frame_space_native = ld_graph->frameSpace;
    original_grid_offset_native = ld_graph->gridOffset;
    backend->kind = TINYUI_BACKEND_WIDGET_BUTTON;

    assert(tinyui_graph_set_axis(graph, 180, 120) == -1);
    assert(graph->x_axis == original_x_axis);
    assert(graph->y_axis == original_y_axis);
    assert(tinyui_graph_set_axis_offset(graph, 17) == -1);
    assert(graph->axis_offset == original_axis_offset_host);
    assert(tinyui_graph_set_frame_space(graph, 19) == -1);
    assert(graph->frame_space == original_frame_space_host);
    assert(tinyui_graph_set_grid_offset(graph, 13) == -1);
    assert(graph->grid_offset == original_grid_offset_host);
    assert(tinyui_graph_set_point_mask_source(
               graph,
               &(struct tinyui_image_source){
                   .img_tile = &g_large_graph_point_mask,
                   .mask_tile = &g_large_graph_point_mask,
               }) == -1);
    assert(graph->point_mask_source == 0);
    assert(tinyui_graph_add_series(graph, 0x2057C4U, 2, 4) == -1);
    assert(tinyui_graph_get_series_count(graph) == -1);
    assert(tinyui_graph_get_value(graph, 0, 0) == -1);

    assert(ld_graph->seriesCount == 0);
    assert(ld_graph->xAxisOffset == original_axis_offset);
    assert(ld_graph->frameSpace == original_frame_space_native);
    assert(ld_graph->gridOffset == original_grid_offset_native);

    backend->kind = original_kind;
    tinyui_app_destroy(app);
}

static void test_graph_point_mask_larger_than_frame_space_is_accepted_and_synced(void)
{
    struct tinyui_app *app;
    struct tinyui_window *win;
    struct tinyui_graph *graph;
    struct tinyui_widget *backend;
    ldGraph_t *ld_graph;
    struct tinyui_image_source large_point_source = {
        .img_tile = &g_large_graph_point_mask,
        .mask_tile = &g_large_graph_point_mask,
    };
    int original_frame_space_host;

    app = tinyui_app_create();
    assert(app != 0);
    win = tinyui_window_create(app, "graph_large_mask_root");
    assert(win != 0);
    graph = tinyui_graph_create(win, "graph_large_mask", 2);
    assert(graph != 0);

    backend = &graph->widget;
    assert(backend->ld_widget != 0);
    ld_graph = (ldGraph_t *)backend->ld_widget;
    assert(ld_graph != 0);

    original_frame_space_host = graph->frame_space;
    assert(original_frame_space_host < g_large_graph_point_mask.tRegion.tSize.iWidth);
    assert(tinyui_graph_set_point_mask_source(graph, &large_point_source) == 0);

    assert(graph->point_mask_source == &large_point_source);
    assert(graph->frame_space == g_large_graph_point_mask.tRegion.tSize.iWidth);
    assert(ld_graph->ptPointMaskTile == large_point_source.mask_tile);
    assert(ld_graph->frameSpace == g_large_graph_point_mask.tRegion.tSize.iWidth);

    tinyui_app_destroy(app);
}

static void test_graph_rejects_null_args(struct tinyui_window *win)
{
    assert(tinyui_graph_create(0, "id", 1) == 0);
    assert(tinyui_graph_create(win, 0, 1) == 0);
    assert(tinyui_graph_add_series(0, 0xFFFFFFU, 1, 10) == -1);
    assert(tinyui_graph_set_value(0, 0, 0, 50) == -1);
    assert(tinyui_graph_get_value(0, 0, 0) == -1);
    assert(tinyui_graph_set_axis(0, 100, 100) == -1);
}

static void test_graph_internal_seams_use_tinyui_prefix(void)
{
    const char *widget_path = "tinyui/src/widgets/graph.c";

    assert_source_lacks_function_definition(widget_path, "tinyui_graph_backend");
    assert_source_lacks_function_definition(widget_path, "tinyui_graph_backend_const");
    assert_source_lacks_function_definition(widget_path, "tinyui_graph_get_ld");
    assert_source_lacks_function_definition(widget_path, "tinyui_graph_get_ld_const");
    assert_source_lacks_function_definition(widget_path, "tinyui_graph_props_are_valid");
    assert_source_lacks_function_definition(widget_path, "tinyui_graph_apply_native_geometry_candidate");

    assert_source_has_function_definition(widget_path, "static int ", "graph_apply_native_geometry_candidate");
}

int main(void)
{
    struct tinyui_app *app = tinyui_app_create();
    struct tinyui_window *win;

    assert(app != 0);
    win = tinyui_window_create(app, "root");
    assert(win != 0);

    test_graph_create_builds_direct_backend_mapping(win);
    test_graph_series_value_readback_survives_frame_update();
    test_graph_visible_output_matches_series_updates();
    test_graph_final_release_contract_covers_advanced_readback_boundary();
    test_graph_native_axis_grid_and_point_mask_round_trip();
    test_graph_move_add_and_set_value_reject_invalid_inputs_without_polluting_other_series();
    test_graph_init_and_shared_base_aliases_round_trip();
    test_graph_rejects_non_graph_backend_binding();
    test_graph_point_mask_larger_than_frame_space_is_accepted_and_synced();
    test_graph_rejects_null_args(win);
    test_graph_internal_seams_use_tinyui_prefix();

    tinyui_app_destroy(app);
    return 0;
}
