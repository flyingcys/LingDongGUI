#include "picoui/picoui.h"
#include "internal.h"
#include "../../../src/gui/ldBase.h"

#include <assert.h>

int picoui_native_event_bind_root(struct picoui_screen *screen, struct picoui_window *root_window);
int picoui_native_graph_get_rendered_series_count(const struct picoui_graph *graph, int *series_count);
int picoui_native_graph_get_rendered_value(const struct picoui_graph *graph,
                                           int series_index,
                                           int value_index,
                                           int *value);

int main(void)
{
    struct picoui_display *display;
    struct picoui_screen *screen;
    struct picoui_window *window;
    struct picoui_graph *graph;
    struct picoui_backend_widget *backend;
    int series;
    int rendered_series_count = -1;
    int rendered_value = -1;
    int rc;

    assert(picoui_init() == 0);

    display = picoui_display_create(320, 480);
    assert(display != 0);
    assert(picoui_display_set_default(display) == 0);

    screen = picoui_screen_active();
    assert(screen != 0);

    window = picoui_window_create_root(screen, "root");
    assert(window != 0);

    graph = picoui_graph_create(window, "traffic", 2);
    assert(graph != 0);
    assert(picoui_widget_set_pos((struct picoui_widget *)graph, 16, 24) == 0);
    assert(picoui_widget_set_size((struct picoui_widget *)graph, 240, 120) == 0);

    series = picoui_graph_add_series(graph, 0x2057C4U, 1, 3);
    assert(series == 0);
    assert(picoui_graph_set_value(graph, series, 0, 12) == 0);
    assert(picoui_graph_set_value(graph, series, 1, 26) == 0);
    assert(picoui_graph_set_value(graph, series, 2, 42) == 0);
    assert(picoui_graph_get_series_count(graph) == 1);
    assert(picoui_graph_get_value(graph, series, 0) == 12);
    assert(picoui_graph_get_value(graph, series, 1) == 26);
    assert(picoui_graph_get_value(graph, series, 2) == 42);

    backend = (struct picoui_backend_widget *)graph->widget.backend_widget;
    assert(backend != 0);
    assert(backend->kind == PICOUI_BACKEND_WIDGET_GRAPH);
    assert(backend->ld_widget != 0);
    assert(ldBaseGetWidgetType((ldBase_t *)backend->ld_widget) == widgetTypeGraph);

    assert(picoui_native_event_bind_root(screen, window) == 0);
    assert(picoui_native_graph_get_rendered_series_count(graph, &rendered_series_count) == -1);
    assert(picoui_native_graph_get_rendered_value(graph, series, 0, &rendered_value) == -1);

    rc = picoui_timer_handler();
    assert(rc == 0 || rc == 1);
    assert(picoui_native_graph_get_rendered_series_count(graph, &rendered_series_count) == 0);
    assert(rendered_series_count == 1);
    assert(picoui_native_graph_get_rendered_value(graph, series, 0, &rendered_value) == 0);
    assert(rendered_value == 12);
    assert(picoui_native_graph_get_rendered_value(graph, series, 1, &rendered_value) == 0);
    assert(rendered_value == 26);
    assert(picoui_native_graph_get_rendered_value(graph, series, 2, &rendered_value) == 0);
    assert(rendered_value == 42);

    picoui_deinit();
    return 0;
}
