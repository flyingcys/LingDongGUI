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
int picoui_graph_set_axis(struct picoui_graph *graph, int x_axis, int y_axis);
int picoui_graph_set_axis_offset(struct picoui_graph *graph, int axis_offset);
int picoui_graph_set_frame_space(struct picoui_graph *graph, int frame_space);
int picoui_graph_set_grid_offset(struct picoui_graph *graph, int grid_offset);
int picoui_graph_set_point_mask_source(struct picoui_graph *graph,
                                       struct picoui_image_source *source);
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
