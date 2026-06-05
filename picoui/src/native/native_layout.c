#include "../core/internal.h"
#include "picoui/layout.h"
#include "picoui/widget.h"
#include "picoui/window.h"

static int picoui_native_layout_supports_window(const struct picoui_window *window)
{
    if (window == 0) {
        return 0;
    }

    if (window->grid_col_count > 0 || window->grid_row_count > 0) {
        return 0;
    }

    if (window->flex_main_align != PICOUI_ALIGN_START
        || window->flex_cross_align != PICOUI_ALIGN_START
        || window->flex_track_align != PICOUI_ALIGN_START) {
        return 0;
    }

    return window->flex_flow == PICOUI_FLEX_FLOW_ROW || window->flex_flow == PICOUI_FLEX_FLOW_COLUMN;
}

static int picoui_native_layout_supports_child(const struct picoui_widget *widget)
{
    if (widget == 0) {
        return 0;
    }

    if (widget->ignore_layout != 0) {
        return 1;
    }

    return widget->flex_grow == 0 && widget->flex_new_track == 0;
}

static int picoui_native_layout_apply_row(struct picoui_window *root_window)
{
    struct picoui_widget *child;
    int cursor_x = 0;

    child = picoui_widget_get_first_child((const struct picoui_widget *)root_window);
    while (child != 0) {
        if (!picoui_native_layout_supports_child(child)) {
            return -1;
        }

        if (child->ignore_layout == 0) {
            if (picoui_widget_set_pos(child, cursor_x, 0) != 0) {
                return -1;
            }
            cursor_x += picoui_widget_get_width(child) + root_window->flex_item_gap;
        }

        child = picoui_widget_get_next_sibling(child);
    }

    return 0;
}

static int picoui_native_layout_sum_tracks(const int *tracks, int from, int span)
{
    int i;
    int total = 0;

    for (i = 0; i < span; ++i) {
        total += tracks[from + i];
    }

    return total;
}

static int picoui_native_layout_grid_position(const int *tracks, int gap, int index)
{
    int i;
    int pos = 0;

    for (i = 0; i < index; ++i) {
        pos += tracks[i] + gap;
    }

    return pos;
}

static int picoui_native_layout_apply_grid(struct picoui_window *root_window)
{
    struct picoui_widget *child;

    if (root_window->grid_col_count <= 0 || root_window->grid_row_count <= 0) {
        return -1;
    }

    child = picoui_widget_get_first_child((const struct picoui_widget *)root_window);
    while (child != 0) {
        int x;
        int y;
        int width;
        int height;

        if (child->ignore_layout != 0) {
            child = picoui_widget_get_next_sibling(child);
            continue;
        }

        if (child->grid_col < 0
            || child->grid_row < 0
            || child->grid_col_span <= 0
            || child->grid_row_span <= 0
            || child->grid_col + child->grid_col_span > root_window->grid_col_count
            || child->grid_row + child->grid_row_span > root_window->grid_row_count) {
            return -1;
        }

        x = picoui_native_layout_grid_position(root_window->grid_cols,
                                               root_window->grid_col_gap,
                                               child->grid_col);
        y = picoui_native_layout_grid_position(root_window->grid_rows,
                                               root_window->grid_row_gap,
                                               child->grid_row);
        width = picoui_native_layout_sum_tracks(root_window->grid_cols,
                                                child->grid_col,
                                                child->grid_col_span)
            + root_window->grid_col_gap * (child->grid_col_span - 1);
        height = picoui_native_layout_sum_tracks(root_window->grid_rows,
                                                 child->grid_row,
                                                 child->grid_row_span)
            + root_window->grid_row_gap * (child->grid_row_span - 1);

        if (picoui_widget_set_pos(child, x, y) != 0 || picoui_widget_set_size(child, width, height) != 0) {
            return -1;
        }

        child = picoui_widget_get_next_sibling(child);
    }

    return 0;
}

static int picoui_native_layout_apply_column(struct picoui_window *root_window)
{
    struct picoui_widget *child;
    int cursor_y = 0;

    child = picoui_widget_get_first_child((const struct picoui_widget *)root_window);
    while (child != 0) {
        if (!picoui_native_layout_supports_child(child)) {
            return -1;
        }

        if (child->ignore_layout == 0) {
            if (picoui_widget_set_pos(child, 0, cursor_y) != 0) {
                return -1;
            }
            cursor_y += picoui_widget_get_height(child) + root_window->flex_item_gap;
        }

        child = picoui_widget_get_next_sibling(child);
    }

    return 0;
}

int picoui_native_layout_apply_root(struct picoui_window *root_window)
{
    if (root_window != 0
        && root_window->grid_col_count > 0
        && root_window->grid_row_count > 0) {
        return picoui_native_layout_apply_grid(root_window);
    }

    if (!picoui_native_layout_supports_window(root_window)) {
        return -1;
    }

    switch (root_window->flex_flow) {
    case PICOUI_FLEX_FLOW_ROW:
        return picoui_native_layout_apply_row(root_window);
    case PICOUI_FLEX_FLOW_COLUMN:
        return picoui_native_layout_apply_column(root_window);
    default:
        return -1;
    }
}
