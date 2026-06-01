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

int main(void)
{
    test_graph_series_value_readback_survives_frame_update();
    test_graph_visible_output_matches_series_updates();
    test_graph_final_release_contract_covers_advanced_readback_boundary();
    return 0;
}
