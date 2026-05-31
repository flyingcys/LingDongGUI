#include "internal.h"
#include "picoui/graph.h"

#include <stdlib.h>

static int picoui_graph_props_are_valid(const struct picoui_graph_props *props)
{
    return props != 0 &&
           props->id != 0 &&
           props->series_max > 0 &&
           props->series_max <= PICOUI_GRAPH_MAX_SERIES &&
           props->width >= 0 &&
           props->height >= 0;
}

struct picoui_graph *picoui_graph_create(struct picoui_window *parent,
                                         const char *id,
                                         int series_max)
{
    struct picoui_graph *graph;

    if (parent == 0 || id == 0 || series_max <= 0 || series_max > PICOUI_GRAPH_MAX_SERIES) {
        return 0;
    }

    graph = calloc(1, sizeof(*graph));
    if (graph == 0) {
        return 0;
    }

    graph->widget.backend_widget =
        picoui_backend_create_graph(parent->widget.backend_widget, id, series_max);
    if (graph->widget.backend_widget == 0) {
        free(graph);
        return 0;
    }

    graph->id = id;
    graph->series_max = series_max;
    graph->widget.visible = 1;
    graph->widget.enabled = 1;
    if (picoui_backend_widget_bind_host(graph->widget.backend_widget, &graph->widget) != 0) {
        free(graph);
        return 0;
    }
    return graph;
}

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

int picoui_graph_add_series(struct picoui_graph *graph,
                            unsigned int series_color,
                            int line_size,
                            int point_max)
{
    int series_index;

    if (graph == 0 || line_size < 0 || point_max <= 0 || point_max > PICOUI_GRAPH_MAX_POINTS) {
        return -1;
    }

    series_index = picoui_backend_graph_add_series(graph->widget.backend_widget,
                                                   series_color,
                                                   line_size,
                                                   point_max);
    if (series_index < 0 || series_index >= PICOUI_GRAPH_MAX_SERIES) {
        return -1;
    }

    graph->series_point_counts[series_index] = point_max;
    if (series_index >= graph->series_count) {
        graph->series_count = series_index + 1;
    }
    return series_index;
}

int picoui_graph_set_value(struct picoui_graph *graph,
                           int series_index,
                           int value_index,
                           int value)
{
    if (graph == 0 || series_index < 0 || series_index >= graph->series_count ||
        value_index < 0 || value_index >= graph->series_point_counts[series_index] ||
        value < 0) {
        return -1;
    }

    if (picoui_backend_graph_set_value(graph->widget.backend_widget, series_index, value_index, value) != 0) {
        return -1;
    }
    return 0;
}

int picoui_graph_move_add(struct picoui_graph *graph, int series_index, int value)
{
    if (graph == 0 || series_index < 0 || series_index >= graph->series_count || value < 0) {
        return -1;
    }

    return picoui_backend_graph_move_add(graph->widget.backend_widget, series_index, value);
}

int picoui_graph_get_series_count(const struct picoui_graph *graph)
{
    if (graph == 0) {
        return -1;
    }

    return picoui_backend_graph_get_series_count((void *)graph->widget.backend_widget);
}

int picoui_graph_get_value(const struct picoui_graph *graph, int series_index, int value_index)
{
    if (graph == 0) {
        return -1;
    }

    return picoui_backend_graph_get_value((void *)graph->widget.backend_widget, series_index, value_index);
}
