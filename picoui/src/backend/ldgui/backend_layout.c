#include "internal.h"

static struct picoui_backend_widget *picoui_backend_window_get(struct picoui_window *window)
{
    if (window == 0) {
        return 0;
    }

    return (struct picoui_backend_widget *)window->widget.backend_widget;
}

static struct picoui_backend_widget *picoui_backend_widget_get(struct picoui_widget *widget)
{
    if (widget == 0) {
        return 0;
    }

    return (struct picoui_backend_widget *)widget->backend_widget;
}

static int picoui_backend_copy_tracks(int *dst, const int *src, int count)
{
    int i;

    if (dst == 0 || src == 0 || count <= 0 || count > PICOUI_BACKEND_LAYOUT_MAX_TRACKS) {
        return -1;
    }

    for (i = 0; i < count; ++i) {
        dst[i] = src[i];
    }
    return 0;
}

int picoui_backend_window_set_flex_flow(struct picoui_window *window, enum picoui_flex_flow flow)
{
    struct picoui_backend_widget *backend_widget = picoui_backend_window_get(window);

    if (backend_widget == 0) {
        return -1;
    }

    backend_widget->window_layout.flex_flow = flow;
    return 0;
}

int picoui_backend_window_set_flex_align(struct picoui_window *window,
                                         enum picoui_align main_align,
                                         enum picoui_align cross_align,
                                         enum picoui_align track_align)
{
    struct picoui_backend_widget *backend_widget = picoui_backend_window_get(window);

    if (backend_widget == 0) {
        return -1;
    }

    backend_widget->window_layout.flex_main_align = main_align;
    backend_widget->window_layout.flex_cross_align = cross_align;
    backend_widget->window_layout.flex_track_align = track_align;
    return 0;
}

int picoui_backend_window_set_flex_gap(struct picoui_window *window, int item_gap, int track_gap)
{
    struct picoui_backend_widget *backend_widget = picoui_backend_window_get(window);

    if (backend_widget == 0) {
        return -1;
    }

    backend_widget->window_layout.flex_item_gap = item_gap;
    backend_widget->window_layout.flex_track_gap = track_gap;
    return 0;
}

int picoui_backend_window_set_grid_columns(struct picoui_window *window, const int *tracks, int count)
{
    struct picoui_backend_widget *backend_widget = picoui_backend_window_get(window);

    if (backend_widget == 0 || picoui_backend_copy_tracks(backend_widget->window_layout.grid_cols, tracks, count) != 0) {
        return -1;
    }

    backend_widget->window_layout.grid_col_count = count;
    return 0;
}

int picoui_backend_window_set_grid_rows(struct picoui_window *window, const int *tracks, int count)
{
    struct picoui_backend_widget *backend_widget = picoui_backend_window_get(window);

    if (backend_widget == 0 || picoui_backend_copy_tracks(backend_widget->window_layout.grid_rows, tracks, count) != 0) {
        return -1;
    }

    backend_widget->window_layout.grid_row_count = count;
    return 0;
}

int picoui_backend_window_set_grid_gap(struct picoui_window *window, int row_gap, int col_gap)
{
    struct picoui_backend_widget *backend_widget = picoui_backend_window_get(window);

    if (backend_widget == 0) {
        return -1;
    }

    backend_widget->window_layout.grid_row_gap = row_gap;
    backend_widget->window_layout.grid_col_gap = col_gap;
    return 0;
}

int picoui_backend_window_set_grid_align(struct picoui_window *window,
                                         enum picoui_align col_align,
                                         enum picoui_align row_align)
{
    struct picoui_backend_widget *backend_widget = picoui_backend_window_get(window);

    if (backend_widget == 0) {
        return -1;
    }

    backend_widget->window_layout.grid_col_align = col_align;
    backend_widget->window_layout.grid_row_align = row_align;
    return 0;
}

int picoui_backend_widget_set_flex_grow(struct picoui_widget *widget, int grow)
{
    struct picoui_backend_widget *backend_widget = picoui_backend_widget_get(widget);

    if (backend_widget == 0 || grow < 0) {
        return -1;
    }

    backend_widget->child_layout.flex_grow = grow;
    return 0;
}

int picoui_backend_widget_set_flex_new_track(struct picoui_widget *widget, int new_track)
{
    struct picoui_backend_widget *backend_widget = picoui_backend_widget_get(widget);

    if (backend_widget == 0) {
        return -1;
    }

    backend_widget->child_layout.flex_new_track = new_track != 0;
    return 0;
}

int picoui_backend_widget_set_ignore_layout(struct picoui_widget *widget, int ignore_layout)
{
    struct picoui_backend_widget *backend_widget = picoui_backend_widget_get(widget);

    if (backend_widget == 0) {
        return -1;
    }

    backend_widget->child_layout.ignore_layout = ignore_layout != 0;
    return 0;
}

int picoui_backend_widget_set_grid_cell(struct picoui_widget *widget,
                                        int col,
                                        int row,
                                        int col_span,
                                        int row_span,
                                        enum picoui_align x_align,
                                        enum picoui_align y_align)
{
    struct picoui_backend_widget *backend_widget = picoui_backend_widget_get(widget);

    if (backend_widget == 0 || col_span <= 0 || row_span <= 0) {
        return -1;
    }

    backend_widget->child_layout.grid_col = col;
    backend_widget->child_layout.grid_row = row;
    backend_widget->child_layout.grid_col_span = col_span;
    backend_widget->child_layout.grid_row_span = row_span;
    backend_widget->child_layout.grid_x_align = x_align;
    backend_widget->child_layout.grid_y_align = y_align;
    return 0;
}
