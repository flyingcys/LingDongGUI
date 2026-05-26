#include "internal.h"
#include "picoui/layout.h"

static int picoui_window_is_valid(struct picoui_window *window)
{
    return window != 0;
}

static int picoui_copy_tracks(int *dst, const int *src, int count)
{
    int i;

    if (src == 0 || count <= 0 || count > PICOUI_LAYOUT_MAX_TRACKS) {
        return -1;
    }

    for (i = 0; i < count; ++i) {
        dst[i] = src[i];
    }
    return 0;
}

int picoui_grid_set_columns(struct picoui_window *window, const int *tracks, int count)
{
    if (!picoui_window_is_valid(window) || picoui_copy_tracks(window->grid_cols, tracks, count) != 0) {
        return -1;
    }

    window->grid_col_count = count;
    return picoui_backend_window_set_grid_columns(window, tracks, count);
}

int picoui_grid_set_rows(struct picoui_window *window, const int *tracks, int count)
{
    if (!picoui_window_is_valid(window) || picoui_copy_tracks(window->grid_rows, tracks, count) != 0) {
        return -1;
    }

    window->grid_row_count = count;
    return picoui_backend_window_set_grid_rows(window, tracks, count);
}

int picoui_grid_set_gap(struct picoui_window *window, int row_gap, int col_gap)
{
    if (!picoui_window_is_valid(window) || row_gap < 0 || col_gap < 0) {
        return -1;
    }

    window->grid_row_gap = row_gap;
    window->grid_col_gap = col_gap;
    return picoui_backend_window_set_grid_gap(window, row_gap, col_gap);
}

int picoui_grid_set_align(struct picoui_window *window,
                          enum picoui_align col_align,
                          enum picoui_align row_align)
{
    if (!picoui_window_is_valid(window)) {
        return -1;
    }

    window->grid_col_align = col_align;
    window->grid_row_align = row_align;
    return picoui_backend_window_set_grid_align(window, col_align, row_align);
}
