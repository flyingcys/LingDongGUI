#include "internal.h"
#include "picoui/widget.h"

static int picoui_widget_is_valid(struct picoui_widget *widget)
{
    return widget != 0;
}

int picoui_widget_set_pos(struct picoui_widget *widget, int x, int y)
{
    if (!picoui_widget_is_valid(widget)) {
        return -1;
    }

    widget->x = x;
    widget->y = y;
    return 0;
}

int picoui_widget_set_size(struct picoui_widget *widget, int width, int height)
{
    if (!picoui_widget_is_valid(widget) || width < 0 || height < 0) {
        return -1;
    }

    widget->width = width;
    widget->height = height;
    return 0;
}

int picoui_widget_set_visible(struct picoui_widget *widget, int visible)
{
    if (!picoui_widget_is_valid(widget)) {
        return -1;
    }

    widget->visible = visible != 0;
    return 0;
}

int picoui_widget_set_enabled(struct picoui_widget *widget, int enabled)
{
    if (!picoui_widget_is_valid(widget)) {
        return -1;
    }

    widget->enabled = enabled != 0;
    return 0;
}

int picoui_widget_set_flex_grow(struct picoui_widget *widget, int grow)
{
    if (!picoui_widget_is_valid(widget) || grow < 0) {
        return -1;
    }

    widget->flex_grow = grow;
    return picoui_backend_widget_set_flex_grow(widget, grow);
}

int picoui_widget_set_flex_new_track(struct picoui_widget *widget, int new_track)
{
    if (!picoui_widget_is_valid(widget)) {
        return -1;
    }

    widget->flex_new_track = new_track != 0;
    return picoui_backend_widget_set_flex_new_track(widget, widget->flex_new_track);
}

int picoui_widget_set_ignore_layout(struct picoui_widget *widget, int ignore_layout)
{
    if (!picoui_widget_is_valid(widget)) {
        return -1;
    }

    widget->ignore_layout = ignore_layout != 0;
    return picoui_backend_widget_set_ignore_layout(widget, widget->ignore_layout);
}

int picoui_widget_set_grid_cell(struct picoui_widget *widget,
                                int col,
                                int row,
                                int col_span,
                                int row_span,
                                enum picoui_align x_align,
                                enum picoui_align y_align)
{
    if (!picoui_widget_is_valid(widget) || col_span <= 0 || row_span <= 0) {
        return -1;
    }

    widget->grid_col = col;
    widget->grid_row = row;
    widget->grid_col_span = col_span;
    widget->grid_row_span = row_span;
    widget->grid_x_align = x_align;
    widget->grid_y_align = y_align;
    return picoui_backend_widget_set_grid_cell(widget, col, row, col_span, row_span, x_align, y_align);
}
