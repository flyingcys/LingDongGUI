#include "picoui/app.h"
#include "picoui/graph.h"
#include "picoui/window.h"
#include "../../../src/gui/ldGraph.h"
#include "backend.h"
#include "internal.h"

#include <assert.h>

static void test_graph_series_value_readback_survives_frame_update(void)
{
    struct picoui_app *app;
    struct picoui_window *win;
    struct picoui_graph *graph;
    struct picoui_backend_widget *backend;
    ldGraph_t *ld_graph;
    int series;

    app = picoui_app_create();
    assert(app != 0);
    win = picoui_window_create(app, "root");
    assert(win != 0);
    graph = picoui_graph_create(win, "graph_readback", 2);
    assert(graph != 0);

    series = picoui_graph_add_series(graph, 0x2057C4U, 2, 4);
    assert(series == 0);
    assert(picoui_graph_set_value(graph, series, 0, 10) == 0);
    assert(picoui_graph_set_value(graph, series, 1, 30) == 0);
    assert(picoui_graph_set_value(graph, series, 2, 55) == 0);
    assert(picoui_graph_set_value(graph, series, 3, 40) == 0);
    assert(picoui_graph_get_series_count(graph) == 1);
    assert(picoui_graph_get_value(graph, series, 2) == 55);

    backend = (struct picoui_backend_widget *)graph->widget.backend_widget;
    assert(backend != 0);
    ld_graph = (ldGraph_t *)backend->ld_widget;
    assert(ld_graph != 0);
    assert(ld_graph->seriesCount == 1);
    assert(ld_graph->pSeries[series].pValueList[2] == 55);
    picoui_app_destroy(app);
}

static void test_graph_visible_output_matches_series_updates(void)
{
    struct picoui_app *app;
    struct picoui_window *win;
    struct picoui_graph *graph;
    struct picoui_backend_widget *backend;
    ldGraph_t *ld_graph;
    int series;

    app = picoui_app_create();
    assert(app != 0);
    win = picoui_window_create(app, "root");
    assert(win != 0);
    graph = picoui_graph_create(win, "graph_move_add", 1);
    assert(graph != 0);

    series = picoui_graph_add_series(graph, 0x418F1FU, 2, 3);
    assert(series == 0);
    assert(picoui_graph_set_value(graph, series, 0, 5) == 0);
    assert(picoui_graph_set_value(graph, series, 1, 15) == 0);
    assert(picoui_graph_set_value(graph, series, 2, 25) == 0);
    assert(picoui_graph_move_add(graph, series, 45) == 0);
    assert(picoui_graph_get_value(graph, series, 0) == 15);
    assert(picoui_graph_get_value(graph, series, 1) == 25);
    assert(picoui_graph_get_value(graph, series, 2) == 45);

    backend = (struct picoui_backend_widget *)graph->widget.backend_widget;
    assert(backend != 0);
    ld_graph = (ldGraph_t *)backend->ld_widget;
    assert(ld_graph != 0);
    assert(ld_graph->pSeries[series].pValueList[0] == 15);
    assert(ld_graph->pSeries[series].pValueList[1] == 25);
    assert(ld_graph->pSeries[series].pValueList[2] == 45);
    picoui_app_destroy(app);
}

static void test_graph_final_release_contract_covers_advanced_readback_boundary(void)
{
    struct picoui_app *app;
    struct picoui_window *win;
    struct picoui_graph *graph;
    struct picoui_backend_widget *backend;
    ldGraph_t *ld_graph;
    int first_series;
    int second_series;

    app = picoui_app_create();
    assert(app != 0);
    win = picoui_window_create(app, "graph_release_root");
    assert(win != 0);
    graph = picoui_graph_create_with_props(
        win,
        &(struct picoui_graph_props){
            .id = "graph_release_ready",
            .series_max = 3,
            .style_class = "chart-card",
            .width = 260,
            .height = 140,
        });
    assert(graph != 0);

    first_series = picoui_graph_add_series(graph, 0x2057C4U, 2, 4);
    second_series = picoui_graph_add_series(graph, 0x41A85FU, 1, 4);
    assert(first_series == 0);
    assert(second_series == 1);
    assert(picoui_graph_set_value(graph, first_series, 0, 8) == 0);
    assert(picoui_graph_set_value(graph, first_series, 1, 16) == 0);
    assert(picoui_graph_set_value(graph, first_series, 2, 32) == 0);
    assert(picoui_graph_set_value(graph, first_series, 3, 48) == 0);
    assert(picoui_graph_set_value(graph, second_series, 0, 5) == 0);
    assert(picoui_graph_set_value(graph, second_series, 1, 10) == 0);
    assert(picoui_graph_set_value(graph, second_series, 2, 15) == 0);
    assert(picoui_graph_set_value(graph, second_series, 3, 20) == 0);

    backend = (struct picoui_backend_widget *)graph->widget.backend_widget;
    assert(backend != 0);
    assert(backend->kind == PICOUI_BACKEND_WIDGET_GRAPH);
    assert(backend->style_class == (const char *)"chart-card");
    ld_graph = (ldGraph_t *)backend->ld_widget;
    assert(ld_graph != 0);

    assert(picoui_graph_get_series_count(graph) == 2);
    assert(picoui_graph_get_value(graph, first_series, 3) == 48);
    assert(picoui_graph_get_value(graph, second_series, 0) == 5);
    assert(ld_graph->seriesCount == 2);
    assert(ld_graph->pSeries[first_series].valueCountMax == 4);
    assert(ld_graph->pSeries[second_series].valueCountMax == 4);
    assert(picoui_graph_add_series(graph, 0xFFFFFFU, -1, 4) == -1);
    assert(picoui_graph_set_value(graph, second_series, 4, 99) == -1);
    assert(picoui_graph_get_value(graph, second_series, 4) == -1);

    picoui_app_destroy(app);
}

static void test_graph_native_axis_grid_and_point_mask_round_trip(void)
{
    struct picoui_app *app;
    struct picoui_window *win;
    struct picoui_graph *graph;
    struct picoui_backend_widget *backend;
    ldGraph_t *ld_graph;
    struct picoui_image_source point_source = {
        .img_tile = (arm_2d_tile_t *)&c_tileWhiteDotMask,
        .mask_tile = (arm_2d_tile_t *)&c_tileWhiteDotMask,
    };
    int expected_extent;

    app = picoui_app_create();
    assert(app != 0);
    win = picoui_window_create(app, "graph_native_root");
    assert(win != 0);
    graph = picoui_graph_create(win, "graph_native_round_trip", 2);
    assert(graph != 0);

    assert(picoui_graph_set_axis(graph, 160, 90) == 0);
    assert(picoui_graph_set_axis_offset(graph, 7) == 0);
    assert(picoui_graph_set_frame_space(graph, 14) == 0);
    assert(picoui_graph_set_grid_offset(graph, 11) == 0);
    assert(picoui_graph_set_point_mask_source(graph, &point_source) == 0);

    backend = (struct picoui_backend_widget *)graph->widget.backend_widget;
    assert(backend != 0);
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

    assert(picoui_graph_set_axis(graph, 0, 90) == -1);
    assert(picoui_graph_set_axis_offset(graph, -1) == -1);
    assert(picoui_graph_set_frame_space(graph, -1) == -1);
    assert(picoui_graph_set_grid_offset(graph, 0) == -1);
    assert(picoui_graph_set_point_mask_source(graph, 0) == -1);

    assert(ld_graph->xAxisMax == (uint16_t)expected_extent);
    assert(ld_graph->yAxisMax == (uint16_t)expected_extent);
    assert(ld_graph->xAxisOffset == 7);
    assert(ld_graph->frameSpace == 14);
    assert(ld_graph->gridOffset == 11);
    assert(ld_graph->ptPointMaskTile == point_source.mask_tile);

    picoui_app_destroy(app);
}

static void test_graph_move_add_and_set_value_reject_invalid_inputs_without_polluting_other_series(void)
{
    struct picoui_app *app;
    struct picoui_window *win;
    struct picoui_graph *graph;
    struct picoui_backend_widget *backend;
    ldGraph_t *ld_graph;
    int first_series;
    int second_series;

    app = picoui_app_create();
    assert(app != 0);
    win = picoui_window_create(app, "graph_series_guard_root");
    assert(win != 0);
    graph = picoui_graph_create(win, "graph_series_guard", 2);
    assert(graph != 0);

    first_series = picoui_graph_add_series(graph, 0x2057C4U, 2, 3);
    second_series = picoui_graph_add_series(graph, 0x41A85FU, 1, 3);
    assert(first_series == 0);
    assert(second_series == 1);

    assert(picoui_graph_set_value(graph, first_series, 0, 11) == 0);
    assert(picoui_graph_set_value(graph, first_series, 1, 22) == 0);
    assert(picoui_graph_set_value(graph, first_series, 2, 33) == 0);
    assert(picoui_graph_set_value(graph, second_series, 0, 7) == 0);
    assert(picoui_graph_set_value(graph, second_series, 1, 14) == 0);
    assert(picoui_graph_set_value(graph, second_series, 2, 21) == 0);

    backend = (struct picoui_backend_widget *)graph->widget.backend_widget;
    assert(backend != 0);
    ld_graph = (ldGraph_t *)backend->ld_widget;
    assert(ld_graph != 0);

    assert(picoui_graph_move_add(graph, first_series, 44) == 0);
    assert(ld_graph->pSeries[first_series].pValueList[0] == 22);
    assert(ld_graph->pSeries[first_series].pValueList[1] == 33);
    assert(ld_graph->pSeries[first_series].pValueList[2] == 44);
    assert(ld_graph->pSeries[second_series].pValueList[0] == 7);
    assert(ld_graph->pSeries[second_series].pValueList[1] == 14);
    assert(ld_graph->pSeries[second_series].pValueList[2] == 21);

    assert(picoui_graph_set_value(graph, second_series, 3, 99) == -1);
    assert(picoui_graph_set_value(graph, second_series, 1, -1) == -1);
    assert(picoui_graph_move_add(graph, second_series, -1) == -1);
    assert(picoui_graph_move_add(graph, 2, 55) == -1);

    assert(ld_graph->pSeries[first_series].pValueList[0] == 22);
    assert(ld_graph->pSeries[first_series].pValueList[1] == 33);
    assert(ld_graph->pSeries[first_series].pValueList[2] == 44);
    assert(ld_graph->pSeries[second_series].pValueList[0] == 7);
    assert(ld_graph->pSeries[second_series].pValueList[1] == 14);
    assert(ld_graph->pSeries[second_series].pValueList[2] == 21);

    picoui_app_destroy(app);
}

static void test_graph_init_and_shared_base_aliases_round_trip(void)
{
    struct picoui_app *app;
    struct picoui_window *win;
    struct picoui_graph *graph;
    struct picoui_backend_widget *backend;
    ldBase_t *ld_base;

    app = picoui_app_create();
    assert(app != 0);
    win = picoui_window_create(app, "graph_base_root");
    assert(win != 0);
    graph = picoui_graph_create(win, "graph_base_aliases", 2);
    assert(graph != 0);
    backend = (struct picoui_backend_widget *)graph->widget.backend_widget;
    assert(backend != 0);
    ld_base = (ldBase_t *)backend->ld_widget;
    assert(ld_base != 0);

    assert(picoui_widget_set_pos(&graph->widget, 17, 29) == 0);
    assert(ld_base->use_as__arm_2d_control_node_t.tRegion.tLocation.iX == 17);
    assert(ld_base->use_as__arm_2d_control_node_t.tRegion.tLocation.iY == 29);
    assert(picoui_widget_set_visible(&graph->widget, 0) == 0);
    assert(picoui_widget_set_opacity(&graph->widget, 81) == 0);
    assert(picoui_widget_set_selectable(&graph->widget, 1) == 0);
    assert(picoui_widget_set_selected(&graph->widget, 1) == 0);
    assert(picoui_widget_set_corner(&graph->widget, 1) == 0);

    assert(ld_base->isHidden == true);
    assert(ld_base->opacity == 81);
    assert(ld_base->isSelectable == true);
    assert(ld_base->isSelected == true);
    assert(ld_base->isCorner == true);
    picoui_app_destroy(app);
}

int main(void)
{
    test_graph_series_value_readback_survives_frame_update();
    test_graph_visible_output_matches_series_updates();
    test_graph_final_release_contract_covers_advanced_readback_boundary();
    test_graph_native_axis_grid_and_point_mask_round_trip();
    test_graph_move_add_and_set_value_reject_invalid_inputs_without_polluting_other_series();
    test_graph_init_and_shared_base_aliases_round_trip();
    return 0;
}
