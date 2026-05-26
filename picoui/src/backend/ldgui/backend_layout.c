#include "backend.h"

static int picoui_backend_window_is_valid(struct picoui_window *window)
{
    return window != 0;
}

static int picoui_backend_widget_is_valid(struct picoui_widget *widget)
{
    return widget != 0;
}

int picoui_backend_window_set_flex_flow(struct picoui_window *window, enum picoui_flex_flow flow)
{
    (void)flow;
    return picoui_backend_window_is_valid(window) ? 0 : -1;
}

int picoui_backend_window_set_flex_align(struct picoui_window *window,
                                         enum picoui_align main_align,
                                         enum picoui_align cross_align,
                                         enum picoui_align track_align)
{
    (void)main_align;
    (void)cross_align;
    (void)track_align;
    return picoui_backend_window_is_valid(window) ? 0 : -1;
}

int picoui_backend_window_set_flex_gap(struct picoui_window *window, int item_gap, int track_gap)
{
    (void)item_gap;
    (void)track_gap;
    return picoui_backend_window_is_valid(window) ? 0 : -1;
}

int picoui_backend_window_set_grid_columns(struct picoui_window *window, const int *tracks, int count)
{
    (void)tracks;
    return picoui_backend_window_is_valid(window) && count > 0 ? 0 : -1;
}

int picoui_backend_window_set_grid_rows(struct picoui_window *window, const int *tracks, int count)
{
    (void)tracks;
    return picoui_backend_window_is_valid(window) && count > 0 ? 0 : -1;
}

int picoui_backend_window_set_grid_gap(struct picoui_window *window, int row_gap, int col_gap)
{
    (void)row_gap;
    (void)col_gap;
    return picoui_backend_window_is_valid(window) ? 0 : -1;
}

int picoui_backend_window_set_grid_align(struct picoui_window *window,
                                         enum picoui_align col_align,
                                         enum picoui_align row_align)
{
    (void)col_align;
    (void)row_align;
    return picoui_backend_window_is_valid(window) ? 0 : -1;
}

int picoui_backend_widget_set_flex_grow(struct picoui_widget *widget, int grow)
{
    (void)grow;
    return picoui_backend_widget_is_valid(widget) ? 0 : -1;
}

int picoui_backend_widget_set_flex_new_track(struct picoui_widget *widget, int new_track)
{
    (void)new_track;
    return picoui_backend_widget_is_valid(widget) ? 0 : -1;
}

int picoui_backend_widget_set_ignore_layout(struct picoui_widget *widget, int ignore_layout)
{
    (void)ignore_layout;
    return picoui_backend_widget_is_valid(widget) ? 0 : -1;
}

int picoui_backend_widget_set_grid_cell(struct picoui_widget *widget,
                                        int col,
                                        int row,
                                        int col_span,
                                        int row_span,
                                        enum picoui_align x_align,
                                        enum picoui_align y_align)
{
    (void)col;
    (void)row;
    (void)col_span;
    (void)row_span;
    (void)x_align;
    (void)y_align;
    return picoui_backend_widget_is_valid(widget) ? 0 : -1;
}
