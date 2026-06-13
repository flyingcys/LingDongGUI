#ifndef TINYUI_LAYOUT_H
#define TINYUI_LAYOUT_H

enum tinyui_align {
    TINYUI_ALIGN_START,
    TINYUI_ALIGN_CENTER,
    TINYUI_ALIGN_END,
    TINYUI_ALIGN_STRETCH,
    TINYUI_ALIGN_SPACE_EVENLY,
    TINYUI_ALIGN_SPACE_AROUND,
    TINYUI_ALIGN_SPACE_BETWEEN,
};

enum tinyui_flex_flow {
    TINYUI_FLEX_FLOW_ROW,
    TINYUI_FLEX_FLOW_COLUMN,
    TINYUI_FLEX_FLOW_ROW_WRAP,
    TINYUI_FLEX_FLOW_COLUMN_WRAP,
    TINYUI_FLEX_FLOW_ROW_REVERSE,
    TINYUI_FLEX_FLOW_COLUMN_REVERSE,
    TINYUI_FLEX_FLOW_ROW_WRAP_REVERSE,
    TINYUI_FLEX_FLOW_COLUMN_WRAP_REVERSE,
};

struct tinyui_window;

int tinyui_flex_set_flow(struct tinyui_window *window, enum tinyui_flex_flow flow);

int tinyui_flex_set_align(struct tinyui_window *window,
                          enum tinyui_align main_align,
                          enum tinyui_align cross_align,
                          enum tinyui_align track_align);

int tinyui_flex_set_gap(struct tinyui_window *window, int item_gap, int track_gap);

int tinyui_grid_set_columns(struct tinyui_window *window, const int *tracks, int count);

int tinyui_grid_set_rows(struct tinyui_window *window, const int *tracks, int count);

int tinyui_grid_set_gap(struct tinyui_window *window, int row_gap, int col_gap);

int tinyui_grid_set_align(struct tinyui_window *window,
                          enum tinyui_align col_align,
                          enum tinyui_align row_align);

#endif
