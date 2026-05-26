#include "internal.h"
#include "ldBase.h"
#include "ldWindow.h"

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

static ldFlexFlow_t picoui_backend_map_flex_flow(enum picoui_flex_flow flow)
{
    switch (flow) {
    case PICOUI_FLEX_FLOW_COLUMN:
        return ldFlexFlowColumn;
    case PICOUI_FLEX_FLOW_ROW_WRAP:
        return ldFlexFlowRowWrap;
    case PICOUI_FLEX_FLOW_COLUMN_WRAP:
        return ldFlexFlowColumnWrap;
    case PICOUI_FLEX_FLOW_ROW_REVERSE:
        return ldFlexFlowRowReverse;
    case PICOUI_FLEX_FLOW_COLUMN_REVERSE:
        return ldFlexFlowColumnReverse;
    case PICOUI_FLEX_FLOW_ROW_WRAP_REVERSE:
        return ldFlexFlowRowWrapReverse;
    case PICOUI_FLEX_FLOW_COLUMN_WRAP_REVERSE:
        return ldFlexFlowColumnWrapReverse;
    case PICOUI_FLEX_FLOW_ROW:
    default:
        return ldFlexFlowRow;
    }
}

static ldFlexMainAlign_t picoui_backend_map_flex_main_align(enum picoui_align align)
{
    switch (align) {
    case PICOUI_ALIGN_CENTER:
        return ldFlexMainAlignCenter;
    case PICOUI_ALIGN_END:
        return ldFlexMainAlignEnd;
    case PICOUI_ALIGN_SPACE_EVENLY:
        return ldFlexMainAlignSpaceEvenly;
    case PICOUI_ALIGN_SPACE_AROUND:
        return ldFlexMainAlignSpaceAround;
    case PICOUI_ALIGN_SPACE_BETWEEN:
        return ldFlexMainAlignSpaceBetween;
    case PICOUI_ALIGN_STRETCH:
    case PICOUI_ALIGN_START:
    default:
        return ldFlexMainAlignStart;
    }
}

static ldFlexCrossAlign_t picoui_backend_map_flex_cross_align(enum picoui_align align)
{
    switch (align) {
    case PICOUI_ALIGN_CENTER:
        return ldFlexCrossAlignCenter;
    case PICOUI_ALIGN_END:
        return ldFlexCrossAlignEnd;
    case PICOUI_ALIGN_STRETCH:
    case PICOUI_ALIGN_SPACE_EVENLY:
    case PICOUI_ALIGN_SPACE_AROUND:
    case PICOUI_ALIGN_SPACE_BETWEEN:
    case PICOUI_ALIGN_START:
    default:
        return ldFlexCrossAlignStart;
    }
}

static ldFlexTrackAlign_t picoui_backend_map_flex_track_align(enum picoui_align align)
{
    switch (align) {
    case PICOUI_ALIGN_CENTER:
        return ldFlexTrackAlignCenter;
    case PICOUI_ALIGN_END:
        return ldFlexTrackAlignEnd;
    case PICOUI_ALIGN_SPACE_BETWEEN:
        return ldFlexTrackAlignSpaceBetween;
    case PICOUI_ALIGN_SPACE_AROUND:
        return ldFlexTrackAlignSpaceAround;
    case PICOUI_ALIGN_SPACE_EVENLY:
        return ldFlexTrackAlignSpaceEvenly;
    case PICOUI_ALIGN_STRETCH:
    case PICOUI_ALIGN_START:
    default:
        return ldFlexTrackAlignStart;
    }
}

static ldGridAlign_t picoui_backend_map_grid_align(enum picoui_align align)
{
    switch (align) {
    case PICOUI_ALIGN_END:
        return ldGridAlignEnd;
    case PICOUI_ALIGN_CENTER:
        return ldGridAlignCenter;
    case PICOUI_ALIGN_STRETCH:
        return ldGridAlignStretch;
    case PICOUI_ALIGN_SPACE_EVENLY:
        return ldGridAlignSpaceEvenly;
    case PICOUI_ALIGN_SPACE_AROUND:
        return ldGridAlignSpaceAround;
    case PICOUI_ALIGN_SPACE_BETWEEN:
        return ldGridAlignSpaceBetween;
    case PICOUI_ALIGN_START:
    default:
        return ldGridAlignStart;
    }
}

static int16_t picoui_backend_map_grid_track(int value)
{
    if (value == 0) {
        return LD_GRID_TEMPLATE_LAST;
    }
    if (value == -2) {
        return LD_GRID_CONTENT;
    }
    if (value < 0) {
        return LD_GRID_FR((-value) - 1);
    }
    return (int16_t)value;
}

static int picoui_backend_copy_tracks(int16_t *dst, const int *src, int count)
{
    int i;

    if (dst == 0 || src == 0 || count <= 0 || count > PICOUI_BACKEND_LAYOUT_MAX_TRACKS) {
        return -1;
    }

    for (i = 0; i < count; ++i) {
        dst[i] = picoui_backend_map_grid_track(src[i]);
    }
    return 0;
}

static ldWindow_t *picoui_backend_get_ld_window(struct picoui_backend_widget *backend_widget)
{
    if (backend_widget == NULL || backend_widget->ld_widget == NULL) {
        return NULL;
    }
    return (ldWindow_t *)backend_widget->ld_widget;
}

static ldBase_t *picoui_backend_get_ld_base(struct picoui_backend_widget *backend_widget)
{
    if (backend_widget == NULL || backend_widget->ld_widget == NULL) {
        return NULL;
    }
    return (ldBase_t *)backend_widget->ld_widget;
}

int picoui_backend_window_set_flex_flow(struct picoui_window *window, enum picoui_flex_flow flow)
{
    struct picoui_backend_widget *backend_widget = picoui_backend_window_get(window);
    ldWindow_t *ld_window;

    if (backend_widget == 0) {
        return -1;
    }

    backend_widget->window_layout.flex_flow = flow;
    ld_window = picoui_backend_get_ld_window(backend_widget);
    if (ld_window != NULL) {
        ldWindowSetFlexFlow(ld_window, picoui_backend_map_flex_flow(flow));
    }
    return 0;
}

int picoui_backend_window_set_flex_align(struct picoui_window *window,
                                         enum picoui_align main_align,
                                         enum picoui_align cross_align,
                                         enum picoui_align track_align)
{
    struct picoui_backend_widget *backend_widget = picoui_backend_window_get(window);
    ldWindow_t *ld_window;

    if (backend_widget == 0) {
        return -1;
    }

    backend_widget->window_layout.flex_main_align = main_align;
    backend_widget->window_layout.flex_cross_align = cross_align;
    backend_widget->window_layout.flex_track_align = track_align;
    ld_window = picoui_backend_get_ld_window(backend_widget);
    if (ld_window != NULL) {
        ldWindowSetFlexAlign(ld_window,
                             picoui_backend_map_flex_main_align(main_align),
                             picoui_backend_map_flex_cross_align(cross_align));
        ldWindowSetFlexTrackAlign(ld_window, picoui_backend_map_flex_track_align(track_align));
    }
    return 0;
}

int picoui_backend_window_set_flex_gap(struct picoui_window *window, int item_gap, int track_gap)
{
    struct picoui_backend_widget *backend_widget = picoui_backend_window_get(window);
    ldWindow_t *ld_window;

    if (backend_widget == 0) {
        return -1;
    }

    backend_widget->window_layout.flex_item_gap = item_gap;
    backend_widget->window_layout.flex_track_gap = track_gap;
    ld_window = picoui_backend_get_ld_window(backend_widget);
    if (ld_window != NULL) {
        ldWindowSetFlexGap(ld_window, (int16_t)item_gap, (int16_t)track_gap);
    }
    return 0;
}

int picoui_backend_window_set_grid_columns(struct picoui_window *window, const int *tracks, int count)
{
    struct picoui_backend_widget *backend_widget = picoui_backend_window_get(window);
    ldWindow_t *ld_window;

    if (backend_widget == 0 || picoui_backend_copy_tracks(backend_widget->window_layout.grid_cols, tracks, count) != 0) {
        return -1;
    }

    backend_widget->window_layout.grid_col_count = count;
    ld_window = picoui_backend_get_ld_window(backend_widget);
    if (ld_window != NULL) {
        ldWindowSetGridDscArray(ld_window,
                                backend_widget->window_layout.grid_cols,
                                backend_widget->window_layout.grid_row_count > 0
                                    ? backend_widget->window_layout.grid_rows
                                    : NULL);
    }
    return 0;
}

int picoui_backend_window_set_grid_rows(struct picoui_window *window, const int *tracks, int count)
{
    struct picoui_backend_widget *backend_widget = picoui_backend_window_get(window);
    ldWindow_t *ld_window;

    if (backend_widget == 0 || picoui_backend_copy_tracks(backend_widget->window_layout.grid_rows, tracks, count) != 0) {
        return -1;
    }

    backend_widget->window_layout.grid_row_count = count;
    ld_window = picoui_backend_get_ld_window(backend_widget);
    if (ld_window != NULL) {
        ldWindowSetGridDscArray(ld_window,
                                backend_widget->window_layout.grid_col_count > 0
                                    ? backend_widget->window_layout.grid_cols
                                    : NULL,
                                backend_widget->window_layout.grid_rows);
    }
    return 0;
}

int picoui_backend_window_set_grid_gap(struct picoui_window *window, int row_gap, int col_gap)
{
    struct picoui_backend_widget *backend_widget = picoui_backend_window_get(window);
    ldWindow_t *ld_window;

    if (backend_widget == 0) {
        return -1;
    }

    backend_widget->window_layout.grid_row_gap = row_gap;
    backend_widget->window_layout.grid_col_gap = col_gap;
    ld_window = picoui_backend_get_ld_window(backend_widget);
    if (ld_window != NULL) {
        ldWindowSetGridGap(ld_window, (int16_t)row_gap, (int16_t)col_gap);
    }
    return 0;
}

int picoui_backend_window_set_grid_align(struct picoui_window *window,
                                         enum picoui_align col_align,
                                         enum picoui_align row_align)
{
    struct picoui_backend_widget *backend_widget = picoui_backend_window_get(window);
    ldWindow_t *ld_window;

    if (backend_widget == 0) {
        return -1;
    }

    backend_widget->window_layout.grid_col_align = col_align;
    backend_widget->window_layout.grid_row_align = row_align;
    ld_window = picoui_backend_get_ld_window(backend_widget);
    if (ld_window != NULL) {
        ldWindowSetGridAlign(ld_window,
                             picoui_backend_map_grid_align(col_align),
                             picoui_backend_map_grid_align(row_align));
    }
    return 0;
}

int picoui_backend_widget_set_flex_grow(struct picoui_widget *widget, int grow)
{
    struct picoui_backend_widget *backend_widget = picoui_backend_widget_get(widget);
    ldBase_t *ld_base;

    if (backend_widget == 0 || grow < 0) {
        return -1;
    }

    backend_widget->child_layout.flex_grow = grow;
    ld_base = picoui_backend_get_ld_base(backend_widget);
    if (ld_base != NULL) {
        ldBaseSetFlexGrow(ld_base, (uint16_t)grow);
    }
    return 0;
}

int picoui_backend_widget_set_flex_new_track(struct picoui_widget *widget, int new_track)
{
    struct picoui_backend_widget *backend_widget = picoui_backend_widget_get(widget);
    ldBase_t *ld_base;

    if (backend_widget == 0) {
        return -1;
    }

    backend_widget->child_layout.flex_new_track = new_track != 0;
    ld_base = picoui_backend_get_ld_base(backend_widget);
    if (ld_base != NULL) {
        ldBaseSetFlexNewTrack(ld_base, new_track != 0);
    }
    return 0;
}

int picoui_backend_widget_set_ignore_layout(struct picoui_widget *widget, int ignore_layout)
{
    struct picoui_backend_widget *backend_widget = picoui_backend_widget_get(widget);
    ldBase_t *ld_base;

    if (backend_widget == 0) {
        return -1;
    }

    backend_widget->child_layout.ignore_layout = ignore_layout != 0;
    ld_base = picoui_backend_get_ld_base(backend_widget);
    if (ld_base != NULL) {
        ldBaseSetIgnoreLayout(ld_base, ignore_layout != 0);
    }
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
    ldBase_t *ld_base;
    ldGridAlign_t grid_x_align;
    ldGridAlign_t grid_y_align;

    if (backend_widget == 0 || col_span <= 0 || row_span <= 0) {
        return -1;
    }

    backend_widget->child_layout.grid_col = col;
    backend_widget->child_layout.grid_row = row;
    backend_widget->child_layout.grid_col_span = col_span;
    backend_widget->child_layout.grid_row_span = row_span;
    backend_widget->child_layout.grid_x_align = x_align;
    backend_widget->child_layout.grid_y_align = y_align;

    ld_base = picoui_backend_get_ld_base(backend_widget);
    if (ld_base != NULL) {
        grid_x_align = picoui_backend_map_grid_align(x_align);
        grid_y_align = picoui_backend_map_grid_align(y_align);
        ldBaseSetGridCell(ld_base,
                          grid_x_align,
                          (int16_t)col,
                          (int16_t)col_span,
                          grid_y_align,
                          (int16_t)row,
                          (int16_t)row_span);
    }
    return 0;
}
