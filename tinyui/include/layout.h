#ifndef TINYUI_LAYOUT_H
#define TINYUI_LAYOUT_H

enum picoui_align {
    PICOUI_ALIGN_START,
    PICOUI_ALIGN_CENTER,
    PICOUI_ALIGN_END,
    PICOUI_ALIGN_STRETCH,
    PICOUI_ALIGN_SPACE_EVENLY,
    PICOUI_ALIGN_SPACE_AROUND,
    PICOUI_ALIGN_SPACE_BETWEEN,
};

enum picoui_flex_flow {
    PICOUI_FLEX_FLOW_ROW,
    PICOUI_FLEX_FLOW_COLUMN,
    PICOUI_FLEX_FLOW_ROW_WRAP,
    PICOUI_FLEX_FLOW_COLUMN_WRAP,
    PICOUI_FLEX_FLOW_ROW_REVERSE,
    PICOUI_FLEX_FLOW_COLUMN_REVERSE,
    PICOUI_FLEX_FLOW_ROW_WRAP_REVERSE,
    PICOUI_FLEX_FLOW_COLUMN_WRAP_REVERSE,
};

struct picoui_window;

int picoui_flex_set_flow(struct picoui_window *window, enum picoui_flex_flow flow);

int picoui_flex_set_align(struct picoui_window *window,
                          enum picoui_align main_align,
                          enum picoui_align cross_align,
                          enum picoui_align track_align);

int picoui_flex_set_gap(struct picoui_window *window, int item_gap, int track_gap);

int picoui_grid_set_columns(struct picoui_window *window, const int *tracks, int count);

int picoui_grid_set_rows(struct picoui_window *window, const int *tracks, int count);

int picoui_grid_set_gap(struct picoui_window *window, int row_gap, int col_gap);

int picoui_grid_set_align(struct picoui_window *window,
                          enum picoui_align col_align,
                          enum picoui_align row_align);

#endif
