#ifndef TINYUI_GRAPH_H
#define TINYUI_GRAPH_H

struct tinyui_window;
struct tinyui_graph;
struct tinyui_image_source;

struct tinyui_graph_props {
    const char *id;
    int series_max;
    int width;
    int height;
    const char *style_class;
    void *user_data;
};

struct tinyui_graph *tinyui_graph_create(struct tinyui_window *parent,
                                         const char *id,
                                         int series_max);

struct tinyui_graph *tinyui_graph_create_with_props(struct tinyui_window *parent,
                                                    const struct tinyui_graph_props *props);

int tinyui_graph_set_axis(struct tinyui_graph *graph, int x_axis, int y_axis);

int tinyui_graph_set_axis_offset(struct tinyui_graph *graph, int axis_offset);

int tinyui_graph_set_frame_space(struct tinyui_graph *graph, int frame_space);

int tinyui_graph_set_grid_offset(struct tinyui_graph *graph, int grid_offset);

int tinyui_graph_set_point_image_mask(struct tinyui_graph *graph,
                                      struct tinyui_image_source *source);

int tinyui_graph_set_point_mask_source(struct tinyui_graph *graph,
                                       struct tinyui_image_source *source);

int tinyui_graph_add_series(struct tinyui_graph *graph,
                            unsigned int series_color,
                            int line_size,
                            int point_max);

int tinyui_graph_set_value(struct tinyui_graph *graph,
                           int series_index,
                           int value_index,
                           int value);

int tinyui_graph_move_add(struct tinyui_graph *graph, int series_index, int value);

int tinyui_graph_get_series_count(const struct tinyui_graph *graph);

int tinyui_graph_get_value(const struct tinyui_graph *graph, int series_index, int value_index);

#endif
