#ifndef PICOUI_GRAPH_H
#define PICOUI_GRAPH_H

#include "picoui/widget.h"

struct picoui_window;
struct picoui_graph;

struct picoui_graph_props {
    const char *id;
    int series_max;
    int width;
    int height;
    const char *style_class;
    void *user_data;
};

struct picoui_graph *picoui_graph_create(struct picoui_window *parent,
                                         const char *id,
                                         int series_max);
struct picoui_graph *picoui_graph_create_with_props(struct picoui_window *parent,
                                                    const struct picoui_graph_props *props);
int picoui_graph_add_series(struct picoui_graph *graph,
                            unsigned int series_color,
                            int line_size,
                            int point_max);
int picoui_graph_set_value(struct picoui_graph *graph,
                           int series_index,
                           int value_index,
                           int value);
int picoui_graph_move_add(struct picoui_graph *graph, int series_index, int value);
int picoui_graph_get_series_count(const struct picoui_graph *graph);
int picoui_graph_get_value(const struct picoui_graph *graph, int series_index, int value_index);

#endif
