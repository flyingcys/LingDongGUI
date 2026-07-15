#ifndef TINYUI_LAYOUT_H
#define TINYUI_LAYOUT_H

#include "core/obj.h"
#include "core/result.h"

#include <stdint.h>

typedef enum tinyui_align {
    TINYUI_ALIGN_START,
    TINYUI_ALIGN_CENTER,
    TINYUI_ALIGN_END,
    TINYUI_ALIGN_STRETCH,
    TINYUI_ALIGN_SPACE_EVENLY,
    TINYUI_ALIGN_SPACE_AROUND,
    TINYUI_ALIGN_SPACE_BETWEEN,
} tinyui_align_t;

typedef enum tinyui_flex_flow {
    TINYUI_FLEX_FLOW_ROW,
    TINYUI_FLEX_FLOW_COLUMN,
    TINYUI_FLEX_FLOW_ROW_WRAP,
    TINYUI_FLEX_FLOW_COLUMN_WRAP,
    TINYUI_FLEX_FLOW_ROW_REVERSE,
    TINYUI_FLEX_FLOW_COLUMN_REVERSE,
    TINYUI_FLEX_FLOW_ROW_WRAP_REVERSE,
    TINYUI_FLEX_FLOW_COLUMN_WRAP_REVERSE,
} tinyui_flex_flow_t;

#define TINYUI_GRID_MAX_TRACKS 16

typedef enum tinyui_grid_unit {
    TINYUI_GRID_UNIT_PX,
    TINYUI_GRID_UNIT_FR,
    TINYUI_GRID_UNIT_CONTENT,
} tinyui_grid_unit_t;

typedef struct tinyui_grid_track tinyui_grid_track_t;

struct tinyui_grid_track {
    tinyui_grid_unit_t unit;
    uint16_t value;
};

tinyui_result_t tinyui_flex_set_flow(tinyui_obj_t *container,
                                     tinyui_flex_flow_t flow);

tinyui_result_t tinyui_flex_set_align(tinyui_obj_t *container,
                                      tinyui_align_t main_align,
                                      tinyui_align_t cross_align,
                                      tinyui_align_t track_align);

tinyui_result_t tinyui_flex_set_gap(tinyui_obj_t *container,
                                    int item_gap,
                                    int track_gap);

tinyui_result_t tinyui_grid_set_columns(tinyui_obj_t *container,
                                        const tinyui_grid_track_t *tracks,
                                        uint8_t count);

tinyui_result_t tinyui_grid_set_rows(tinyui_obj_t *container,
                                     const tinyui_grid_track_t *tracks,
                                     uint8_t count);

tinyui_result_t tinyui_grid_set_gap(tinyui_obj_t *container,
                                    int row_gap,
                                    int col_gap);

tinyui_result_t tinyui_grid_set_align(tinyui_obj_t *container,
                                      tinyui_align_t col_align,
                                      tinyui_align_t row_align);

/* Child layout item attributes (flex/grid participants). */
tinyui_result_t tinyui_obj_set_flex_grow(tinyui_obj_t *obj, int grow);
tinyui_result_t tinyui_obj_set_flex_new_track(tinyui_obj_t *obj, int new_track);
tinyui_result_t tinyui_obj_set_flex_min_width(tinyui_obj_t *obj, int min_width);
tinyui_result_t tinyui_obj_set_flex_min_height(tinyui_obj_t *obj, int min_height);
tinyui_result_t tinyui_obj_set_flex_max_width(tinyui_obj_t *obj, int max_width);
tinyui_result_t tinyui_obj_set_flex_max_height(tinyui_obj_t *obj, int max_height);
tinyui_result_t tinyui_obj_set_ignore_layout(tinyui_obj_t *obj, int ignore_layout);
tinyui_result_t tinyui_obj_set_grid_cell(tinyui_obj_t *obj,
                                         int col,
                                         int row,
                                         int col_span,
                                         int row_span,
                                         tinyui_align_t x_align,
                                         tinyui_align_t y_align);

#endif
