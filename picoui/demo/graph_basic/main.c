#include "picoui/picoui.h"

static void make_ui(struct picoui_window *win)
{
    struct picoui_label *title;
    struct picoui_graph *graph;
    int cpu_series;
    static const int cols[] = {280, 0};
    static const int rows[] = {28, 120, 0};

    picoui_grid_set_columns(win, cols, 2);
    picoui_grid_set_rows(win, rows, 3);
    picoui_grid_set_gap(win, 12, 12);
    picoui_grid_set_align(win, PICOUI_ALIGN_START, PICOUI_ALIGN_START);
    picoui_window_set_padding_group(win, 24, 24, 24, 24);

    title = picoui_label_create(win, "title");
    graph = picoui_graph_create_with_props(
        win,
        &(struct picoui_graph_props){
            .id = "graph",
            .series_max = 1,
            .width = 180,
            .height = 96,
        });

    if (title != 0) {
        picoui_label_set_text(title, "Graph");
        picoui_widget_set_size((struct picoui_widget *)title, 220, 28);
        picoui_widget_set_grid_cell((struct picoui_widget *)title,
                                    0, 0, 1, 1,
                                    PICOUI_ALIGN_START,
                                    PICOUI_ALIGN_CENTER);
    }

    if (graph != 0) {
        cpu_series = picoui_graph_add_series(graph, 0x2057C4U, 1, 3);
        if (cpu_series >= 0) {
            picoui_graph_set_value(graph, cpu_series, 0, 12);
            picoui_graph_set_value(graph, cpu_series, 1, 26);
            picoui_graph_set_value(graph, cpu_series, 2, 42);
        }
        picoui_widget_set_grid_cell((struct picoui_widget *)graph,
                                    0, 1, 1, 1,
                                    PICOUI_ALIGN_START,
                                    PICOUI_ALIGN_CENTER);
    }
}

int main(void)
{
    struct picoui_app *app;
    struct picoui_window *win;

    app = picoui_app_create();
    if (app == 0) {
        return 1;
    }

    win = picoui_window_create(app, "root");
    if (win == 0) {
        picoui_app_destroy(app);
        return 1;
    }

    make_ui(win);
    return picoui_app_run(app, win);
}
