#include "../backend/ldgui/backend.h"
#include "../core/internal.h"
#include "picoui/graph.h"

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

int picoui_native_graph_render(const struct picoui_backend_widget *backend)
{
    struct picoui_graph *graph;
    struct picoui_graph_ext *ext;
    int series_index;
    int value_index;
    int series_count;
    int value;

    if (backend == 0 || backend->kind != PICOUI_BACKEND_WIDGET_GRAPH || backend->host_widget == 0) {
        return -1;
    }

    graph = (struct picoui_graph *)backend->host_widget;
    ext = picoui_graph_ext_from_graph(graph);
    if (ext == 0) {
        return -1;
    }

    series_count = picoui_graph_get_series_count(graph);
    if (series_count < 0 || series_count > PICOUI_GRAPH_MAX_SERIES) {
        return -1;
    }

    ext->rendered_series_count = series_count;
    for (series_index = 0; series_index < series_count; ++series_index) {
        ext->rendered_series_point_counts[series_index] = graph->series_point_counts[series_index];
        for (value_index = 0; value_index < graph->series_point_counts[series_index]; ++value_index) {
            value = picoui_graph_get_value(graph, series_index, value_index);
            if (value < 0) {
                return -1;
            }
            ext->rendered_values[series_index][value_index] = value;
        }
    }

    ext->render_ready = 1;
    return 0;
}

int picoui_native_graph_get_rendered_series_count(const struct picoui_graph *graph, int *series_count)
{
    const struct picoui_graph_ext *ext;

    if (series_count == 0) {
        return -1;
    }

    ext = picoui_graph_ext_from_graph_const(graph);
    if (ext == 0 || ext->render_ready == 0) {
        return -1;
    }

    *series_count = ext->rendered_series_count;
    return 0;
}

int picoui_native_graph_get_rendered_value(const struct picoui_graph *graph,
                                           int series_index,
                                           int value_index,
                                           int *value)
{
    const struct picoui_graph_ext *ext;

    if (value == 0) {
        return -1;
    }

    ext = picoui_graph_ext_from_graph_const(graph);
    if (ext == 0 || ext->render_ready == 0 ||
        series_index < 0 || series_index >= ext->rendered_series_count ||
        value_index < 0 || value_index >= ext->rendered_series_point_counts[series_index]) {
        return -1;
    }

    *value = ext->rendered_values[series_index][value_index];
    return 0;
}
