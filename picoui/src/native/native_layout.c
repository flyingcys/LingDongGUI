#include "../core/internal.h"
#include "picoui/layout.h"
#include "picoui/widget.h"
#include "picoui/window.h"

static int picoui_native_layout_resolve_extent(int requested, int available)
{
    if (requested > 0 && requested <= available) {
        return requested;
    }

    return available;
}

static int picoui_native_layout_align_offset(enum picoui_align align, int available, int requested);

static int picoui_native_layout_supports_window(const struct picoui_window *window)
{
    if (window == 0) {
        return 0;
    }

    if (window->grid_col_count > 0 || window->grid_row_count > 0) {
        return 0;
    }

    if ((window->flex_main_align != PICOUI_ALIGN_START
         && window->flex_main_align != PICOUI_ALIGN_CENTER
         && window->flex_main_align != PICOUI_ALIGN_END)
        || (window->flex_cross_align != PICOUI_ALIGN_START
            && window->flex_cross_align != PICOUI_ALIGN_CENTER
            && window->flex_cross_align != PICOUI_ALIGN_END
            && window->flex_cross_align != PICOUI_ALIGN_STRETCH)) {
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
    int content_width;
    int content_height;
    int cursor_x;
    int padding_left;
    int padding_top;
    int padding_right;
    int padding_bottom;
    int child_count = 0;
    int total_width = 0;

    padding_left = picoui_window_get_padding_left(root_window);
    padding_top = picoui_window_get_padding_top(root_window);
    padding_right = picoui_window_get_padding_right(root_window);
    padding_bottom = picoui_window_get_padding_bottom(root_window);
    if (padding_left < 0 || padding_top < 0 || padding_right < 0 || padding_bottom < 0) {
        return -1;
    }

    content_width = picoui_widget_get_width((const struct picoui_widget *)root_window)
        - padding_left - padding_right;
    content_height = picoui_widget_get_height((const struct picoui_widget *)root_window)
        - padding_top - padding_bottom;
    if (content_width < 0 || content_height < 0) {
        return -1;
    }

    child = picoui_widget_get_first_child((const struct picoui_widget *)root_window);
    while (child != 0) {
        if (!picoui_native_layout_supports_child(child)) {
            return -1;
        }

        if (child->ignore_layout == 0) {
            total_width += picoui_widget_get_width(child);
            child_count += 1;
        }

        child = picoui_widget_get_next_sibling(child);
    }

    if (child_count > 1) {
        total_width += root_window->flex_item_gap * (child_count - 1);
    }

    cursor_x = padding_left + picoui_native_layout_align_offset(root_window->flex_main_align,
                                                                content_width,
                                                                total_width);

    child = picoui_widget_get_first_child((const struct picoui_widget *)root_window);
    while (child != 0) {
        int child_width;
        int child_height;
        int child_y;

        if (!picoui_native_layout_supports_child(child)) {
            return -1;
        }

        if (child->ignore_layout == 0) {
            child_width = picoui_widget_get_width(child);
            child_height = picoui_widget_get_height(child);
            if (root_window->flex_cross_align == PICOUI_ALIGN_STRETCH) {
                child_height = content_height;
                if (picoui_widget_set_size(child, child_width, child_height) != 0) {
                    return -1;
                }
            }

            child_y = padding_top + picoui_native_layout_align_offset(root_window->flex_cross_align,
                                                                      content_height,
                                                                      child_height);
            if (picoui_widget_set_pos(child, cursor_x, child_y) != 0) {
                return -1;
            }
            cursor_x += child_width + root_window->flex_item_gap;
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

static int picoui_native_layout_align_offset(enum picoui_align align, int available, int requested)
{
    if (available <= requested) {
        return 0;
    }

    switch (align) {
    case PICOUI_ALIGN_CENTER:
        return (available - requested) / 2;
    case PICOUI_ALIGN_END:
        return available - requested;
    case PICOUI_ALIGN_STRETCH:
    case PICOUI_ALIGN_START:
    default:
        return 0;
    }
}

static int picoui_native_layout_apply_grid(struct picoui_window *root_window)
{
    struct picoui_widget *child;
    int padding_left;
    int padding_top;

    if (root_window->grid_col_count <= 0 || root_window->grid_row_count <= 0) {
        return -1;
    }

    padding_left = picoui_window_get_padding_left(root_window);
    padding_top = picoui_window_get_padding_top(root_window);
    if (padding_left < 0 || padding_top < 0) {
        return -1;
    }

    child = picoui_widget_get_first_child((const struct picoui_widget *)root_window);
    while (child != 0) {
        int x;
        int y;
        int width;
        int height;
        int requested_width;
        int requested_height;

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
                                               child->grid_col)
            + padding_left;
        y = picoui_native_layout_grid_position(root_window->grid_rows,
                                               root_window->grid_row_gap,
                                               child->grid_row)
            + padding_top;
        width = picoui_native_layout_sum_tracks(root_window->grid_cols,
                                                child->grid_col,
                                                child->grid_col_span)
            + root_window->grid_col_gap * (child->grid_col_span - 1);
        height = picoui_native_layout_sum_tracks(root_window->grid_rows,
                                                 child->grid_row,
                                                 child->grid_row_span)
            + root_window->grid_row_gap * (child->grid_row_span - 1);

        requested_width = picoui_widget_get_width(child);
        requested_height = picoui_widget_get_height(child);
        width = picoui_native_layout_resolve_extent(requested_width, width);
        height = picoui_native_layout_resolve_extent(requested_height, height);
        x += picoui_native_layout_align_offset(child->grid_x_align, picoui_native_layout_sum_tracks(root_window->grid_cols,
                                                                                                    child->grid_col,
                                                                                                    child->grid_col_span)
                                                                 + root_window->grid_col_gap * (child->grid_col_span - 1),
                                               width);
        y += picoui_native_layout_align_offset(child->grid_y_align, picoui_native_layout_sum_tracks(root_window->grid_rows,
                                                                                                    child->grid_row,
                                                                                                    child->grid_row_span)
                                                                 + root_window->grid_row_gap * (child->grid_row_span - 1),
                                               height);

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
    int content_width;
    int content_height;
    int cursor_y;
    int padding_left;
    int padding_top;
    int padding_right;
    int padding_bottom;
    int child_count = 0;
    int total_height = 0;

    padding_left = picoui_window_get_padding_left(root_window);
    padding_top = picoui_window_get_padding_top(root_window);
    padding_right = picoui_window_get_padding_right(root_window);
    padding_bottom = picoui_window_get_padding_bottom(root_window);
    if (padding_left < 0 || padding_top < 0 || padding_right < 0 || padding_bottom < 0) {
        return -1;
    }

    content_width = picoui_widget_get_width((const struct picoui_widget *)root_window)
        - padding_left - padding_right;
    content_height = picoui_widget_get_height((const struct picoui_widget *)root_window)
        - padding_top - padding_bottom;
    if (content_width < 0 || content_height < 0) {
        return -1;
    }

    child = picoui_widget_get_first_child((const struct picoui_widget *)root_window);
    while (child != 0) {
        if (!picoui_native_layout_supports_child(child)) {
            return -1;
        }

        if (child->ignore_layout == 0) {
            total_height += picoui_widget_get_height(child);
            child_count += 1;
        }

        child = picoui_widget_get_next_sibling(child);
    }

    if (child_count > 1) {
        total_height += root_window->flex_item_gap * (child_count - 1);
    }

    cursor_y = padding_top + picoui_native_layout_align_offset(root_window->flex_main_align,
                                                               content_height,
                                                               total_height);

    child = picoui_widget_get_first_child((const struct picoui_widget *)root_window);
    while (child != 0) {
        int child_width;
        int child_height;
        int child_x;

        if (!picoui_native_layout_supports_child(child)) {
            return -1;
        }

        if (child->ignore_layout == 0) {
            child_width = picoui_widget_get_width(child);
            child_height = picoui_widget_get_height(child);
            if (root_window->flex_cross_align == PICOUI_ALIGN_STRETCH) {
                child_width = content_width;
                if (picoui_widget_set_size(child, child_width, child_height) != 0) {
                    return -1;
                }
            }

            child_x = padding_left + picoui_native_layout_align_offset(root_window->flex_cross_align,
                                                                       content_width,
                                                                       child_width);
            if (picoui_widget_set_pos(child, child_x, cursor_y) != 0) {
                return -1;
            }
            cursor_y += child_height + root_window->flex_item_gap;
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
