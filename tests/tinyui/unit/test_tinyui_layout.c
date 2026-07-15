/*
 * TinyUI M3 Task 7 — direct flex/grid mapping unit tests.
 *
 * Typed track conversion, parameter rejection, real ldWindow layout fields,
 * and zero allocator delta. Does not treat TinyUI grid arrays as truth.
 */

#include "tinyui.h"
#include "internal.h"
#include "ldBase.h"
#include "ldWindow.h"
#include "tinyui_test_support.h"

#include <assert.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

static tinyui_obj_t *test_boot_screen(void)
{
    tinyui_obj_t *screen;

    assert(tinyui_init() == TINYUI_OK);
    screen = tinyui_screen_create();
    assert(screen != 0);
    return screen;
}

static void test_shutdown(void)
{
    tinyui_deinit();
}

static ldWindow_t *ld_window_of(tinyui_obj_t *obj)
{
    struct tinyui_widget *widget = (struct tinyui_widget *)(void *)obj;

    assert(widget != 0);
    assert(widget->ld_widget != 0);
    return (ldWindow_t *)widget->ld_widget;
}

static ldBase_t *ld_base_of(tinyui_obj_t *obj)
{
    struct tinyui_widget *widget = (struct tinyui_widget *)(void *)obj;

    assert(widget != 0);
    assert(widget->ld_widget != 0);
    return (ldBase_t *)widget->ld_widget;
}

static void test_flex_rejects_invalid_object_and_flow(void)
{
    tinyui_obj_t *screen = test_boot_screen();
    tinyui_obj_t *button = tinyui_button_create(screen);
    ldWindow_t *ld_window = ld_window_of(screen);

    assert(button != 0);
    assert(tinyui_flex_set_flow(0, TINYUI_FLEX_FLOW_ROW) == TINYUI_ERROR_INVALID_OBJECT);
    assert(tinyui_flex_set_flow(button, TINYUI_FLEX_FLOW_ROW) == TINYUI_ERROR_INVALID_OBJECT);
    assert(tinyui_flex_set_flow(screen, (tinyui_flex_flow_t)99) == TINYUI_ERROR_INVALID_ARG);
    assert(ld_window->layoutTpye != layoutFlex);

    test_shutdown();
}

static void test_flex_flow_align_gap_map_to_ld_window(void)
{
    tinyui_obj_t *screen = test_boot_screen();
    ldWindow_t *ld_window = ld_window_of(screen);
    struct tinyui_test_allocator_stats before;
    struct tinyui_test_allocator_stats after;

    tinyui_test_allocator_reset();
    before = tinyui_test_allocator_snapshot();

    assert(tinyui_flex_set_flow(screen, TINYUI_FLEX_FLOW_ROW_WRAP) == TINYUI_OK);
    assert(tinyui_flex_set_align(screen,
                                 TINYUI_ALIGN_START,
                                 TINYUI_ALIGN_CENTER,
                                 TINYUI_ALIGN_SPACE_BETWEEN) == TINYUI_OK);
    assert(tinyui_flex_set_gap(screen, 8, 12) == TINYUI_OK);

    after = tinyui_test_allocator_snapshot();
    assert(after.alloc_calls == before.alloc_calls);
    assert(after.calloc_calls == before.calloc_calls);
    assert(after.realloc_calls == before.realloc_calls);

    assert(ld_window->layoutTpye == layoutFlex);
    assert(ld_window->flexFlow == ldFlexFlowRowWrap);
    assert(ld_window->flexMainAlign == ldFlexMainAlignStart);
    assert(ld_window->flexCrossAlign == ldFlexCrossAlignCenter);
    assert(ld_window->flexTrackAlign == ldFlexTrackAlignSpaceBetween);
    assert(ld_window->flexItemGap == 8);
    assert(ld_window->flexTrackGap == 12);

    assert(tinyui_flex_set_flow(screen, TINYUI_FLEX_FLOW_COLUMN) == TINYUI_OK);
    assert(ld_window->flexFlow == ldFlexFlowColumn);
    assert(tinyui_flex_set_flow(screen, TINYUI_FLEX_FLOW_COLUMN_WRAP) == TINYUI_OK);
    assert(ld_window->flexFlow == ldFlexFlowColumnWrap);
    assert(tinyui_flex_set_flow(screen, TINYUI_FLEX_FLOW_ROW_REVERSE) == TINYUI_OK);
    assert(ld_window->flexFlow == ldFlexFlowRowReverse);
    assert(tinyui_flex_set_flow(screen, TINYUI_FLEX_FLOW_COLUMN_REVERSE) == TINYUI_OK);
    assert(ld_window->flexFlow == ldFlexFlowColumnReverse);
    assert(tinyui_flex_set_flow(screen, TINYUI_FLEX_FLOW_ROW_WRAP_REVERSE) == TINYUI_OK);
    assert(ld_window->flexFlow == ldFlexFlowRowWrapReverse);
    assert(tinyui_flex_set_flow(screen, TINYUI_FLEX_FLOW_COLUMN_WRAP_REVERSE) == TINYUI_OK);
    assert(ld_window->flexFlow == ldFlexFlowColumnWrapReverse);
    assert(tinyui_flex_set_flow(screen, TINYUI_FLEX_FLOW_ROW) == TINYUI_OK);
    assert(ld_window->flexFlow == ldFlexFlowRow);

    assert(tinyui_flex_set_align(screen,
                                 TINYUI_ALIGN_END,
                                 TINYUI_ALIGN_END,
                                 TINYUI_ALIGN_END) == TINYUI_OK);
    assert(ld_window->flexMainAlign == ldFlexMainAlignEnd);
    assert(ld_window->flexCrossAlign == ldFlexCrossAlignEnd);
    assert(ld_window->flexTrackAlign == ldFlexTrackAlignEnd);

    assert(tinyui_flex_set_align(screen,
                                 TINYUI_ALIGN_SPACE_EVENLY,
                                 TINYUI_ALIGN_START,
                                 TINYUI_ALIGN_SPACE_AROUND) == TINYUI_OK);
    assert(ld_window->flexMainAlign == ldFlexMainAlignSpaceEvenly);
    assert(ld_window->flexTrackAlign == ldFlexTrackAlignSpaceAround);

    assert(tinyui_flex_set_align(screen,
                                 TINYUI_ALIGN_SPACE_AROUND,
                                 TINYUI_ALIGN_STRETCH,
                                 TINYUI_ALIGN_SPACE_EVENLY) == TINYUI_OK);
    assert(ld_window->flexMainAlign == ldFlexMainAlignSpaceAround);
    assert(ld_window->flexCrossAlign == ldFlexCrossAlignStart);
    assert(ld_window->flexTrackAlign == ldFlexTrackAlignSpaceEvenly);

    assert(tinyui_flex_set_align(screen,
                                 TINYUI_ALIGN_SPACE_BETWEEN,
                                 TINYUI_ALIGN_CENTER,
                                 TINYUI_ALIGN_START) == TINYUI_OK);
    assert(ld_window->flexMainAlign == ldFlexMainAlignSpaceBetween);

    assert(tinyui_flex_set_align(screen,
                                 (tinyui_align_t)99,
                                 TINYUI_ALIGN_CENTER,
                                 TINYUI_ALIGN_START) == TINYUI_ERROR_INVALID_ARG);
    assert(tinyui_flex_set_gap(screen, -1, 0) == TINYUI_ERROR_OUT_OF_RANGE);
    assert(tinyui_flex_set_gap(screen, 0, -1) == TINYUI_ERROR_OUT_OF_RANGE);
    assert(tinyui_flex_set_gap(screen, (int)INT16_MAX + 1, 0) == TINYUI_ERROR_OUT_OF_RANGE);
    assert(ld_window->flexItemGap == 8);
    assert(ld_window->flexTrackGap == 12);

    test_shutdown();
}

static void test_flex_child_grow_minmax_new_track_ignore_layout(void)
{
    tinyui_obj_t *screen = test_boot_screen();
    tinyui_obj_t *a = tinyui_button_create(screen);
    tinyui_obj_t *b = tinyui_button_create(screen);
    ldBase_t *ld_a = ld_base_of(a);
    ldBase_t *ld_b = ld_base_of(b);

    assert(tinyui_flex_set_flow(screen, TINYUI_FLEX_FLOW_ROW) == TINYUI_OK);
    assert(tinyui_obj_set_flex_grow(a, 2) == TINYUI_OK);
    assert(tinyui_obj_set_flex_new_track(a, 1) == TINYUI_OK);
    assert(tinyui_obj_set_flex_min_width(a, 24) == TINYUI_OK);
    assert(tinyui_obj_set_flex_min_height(a, 12) == TINYUI_OK);
    assert(tinyui_obj_set_flex_max_width(a, 88) == TINYUI_OK);
    assert(tinyui_obj_set_flex_max_height(a, 42) == TINYUI_OK);
    assert(tinyui_obj_set_ignore_layout(b, 1) == TINYUI_OK);

    assert(ld_a->flexGrow == 2);
    assert(ld_a->flexInNewTrack == true);
    assert(ld_a->hasFlexMinWidth == true);
    assert(ld_a->hasFlexMinHeight == true);
    assert(ld_a->hasFlexMaxWidth == true);
    assert(ld_a->hasFlexMaxHeight == true);
    assert(ld_a->flexMinSize.iWidth == 24);
    assert(ld_a->flexMinSize.iHeight == 12);
    assert(ld_a->flexMaxSize.iWidth == 88);
    assert(ld_a->flexMaxSize.iHeight == 42);
    assert(ld_b->ignoreLayout == true);

    test_shutdown();
}

static void test_grid_typed_tracks_map_to_ld_window(void)
{
    tinyui_obj_t *screen = test_boot_screen();
    ldWindow_t *ld_window = ld_window_of(screen);
    const tinyui_grid_track_t cols[] = {
        { TINYUI_GRID_UNIT_PX, 80 },
        { TINYUI_GRID_UNIT_FR, 1 },
        { TINYUI_GRID_UNIT_CONTENT, 0 },
    };
    const tinyui_grid_track_t rows[] = {
        { TINYUI_GRID_UNIT_PX, 24 },
        { TINYUI_GRID_UNIT_FR, 2 },
    };
    struct tinyui_test_allocator_stats before;
    struct tinyui_test_allocator_stats after;

    tinyui_test_allocator_reset();
    before = tinyui_test_allocator_snapshot();

    assert(tinyui_grid_set_columns(screen, cols, 3) == TINYUI_OK);
    assert(tinyui_grid_set_rows(screen, rows, 2) == TINYUI_OK);
    assert(tinyui_grid_set_gap(screen, 6, 10) == TINYUI_OK);
    assert(tinyui_grid_set_align(screen, TINYUI_ALIGN_END, TINYUI_ALIGN_SPACE_AROUND)
           == TINYUI_OK);

    after = tinyui_test_allocator_snapshot();
    assert(after.alloc_calls == before.alloc_calls);
    assert(after.calloc_calls == before.calloc_calls);
    assert(after.realloc_calls == before.realloc_calls);

    assert(ld_window->layoutTpye == layoutGrid);
    assert(ld_window->gridColDsc != 0);
    assert(ld_window->gridRowDsc != 0);
    assert(ld_window->gridColDsc[0] == 80);
    assert(ld_window->gridColDsc[1] == LD_GRID_FR(1));
    assert(ld_window->gridColDsc[2] == LD_GRID_CONTENT);
    assert(ld_window->gridColDsc[3] == LD_GRID_TEMPLATE_LAST);
    assert(ld_window->gridRowDsc[0] == 24);
    assert(ld_window->gridRowDsc[1] == LD_GRID_FR(2));
    assert(ld_window->gridRowDsc[2] == LD_GRID_TEMPLATE_LAST);
    assert(ld_window->gridRowGap == 6);
    assert(ld_window->gridColumnGap == 10);
    assert(ld_window->gridColAlign == ldGridAlignEnd);
    assert(ld_window->gridRowAlign == ldGridAlignSpaceAround);

    {
        const tinyui_grid_track_t short_cols[] = {
            { TINYUI_GRID_UNIT_PX, 64 },
        };
        assert(tinyui_grid_set_columns(screen, short_cols, 1) == TINYUI_OK);
        assert(ld_window->gridColDsc[0] == 64);
        assert(ld_window->gridColDsc[1] == LD_GRID_TEMPLATE_LAST);
    }

    test_shutdown();
}

static void test_grid_parameter_conversion_failures(void)
{
    tinyui_obj_t *screen = test_boot_screen();
    ldWindow_t *ld_window = ld_window_of(screen);
    const tinyui_grid_track_t valid[] = {
        { TINYUI_GRID_UNIT_PX, 40 },
        { TINYUI_GRID_UNIT_FR, 1 },
    };
    tinyui_grid_track_t too_many[TINYUI_GRID_MAX_TRACKS + 1];
    unsigned int i;

    assert(tinyui_grid_set_columns(0, valid, 2) == TINYUI_ERROR_INVALID_OBJECT);
    assert(tinyui_grid_set_columns(screen, 0, 1) == TINYUI_ERROR_INVALID_ARG);
    assert(tinyui_grid_set_columns(screen, valid, 0) == TINYUI_ERROR_OUT_OF_RANGE);

    for (i = 0; i < TINYUI_GRID_MAX_TRACKS + 1U; ++i) {
        too_many[i].unit = TINYUI_GRID_UNIT_PX;
        too_many[i].value = 8;
    }
    assert(tinyui_grid_set_columns(screen, too_many, TINYUI_GRID_MAX_TRACKS + 1)
           == TINYUI_ERROR_OUT_OF_RANGE);

    assert(tinyui_grid_set_columns(screen,
                                   &(const tinyui_grid_track_t){ TINYUI_GRID_UNIT_PX, 0 },
                                   1) == TINYUI_ERROR_OUT_OF_RANGE);
    assert(tinyui_grid_set_columns(screen,
                                   &(const tinyui_grid_track_t){ TINYUI_GRID_UNIT_PX, 32768 },
                                   1) == TINYUI_ERROR_OUT_OF_RANGE);
    assert(tinyui_grid_set_columns(screen,
                                   &(const tinyui_grid_track_t){ TINYUI_GRID_UNIT_FR, 0 },
                                   1) == TINYUI_ERROR_OUT_OF_RANGE);
    assert(tinyui_grid_set_columns(screen,
                                   &(const tinyui_grid_track_t){ TINYUI_GRID_UNIT_FR, 256 },
                                   1) == TINYUI_ERROR_OUT_OF_RANGE);
    assert(tinyui_grid_set_columns(screen,
                                   &(const tinyui_grid_track_t){ TINYUI_GRID_UNIT_CONTENT, 1 },
                                   1) == TINYUI_ERROR_OUT_OF_RANGE);
    assert(tinyui_grid_set_columns(screen,
                                   &(const tinyui_grid_track_t){ (tinyui_grid_unit_t)9, 1 },
                                   1) == TINYUI_ERROR_OUT_OF_RANGE);

    {
        const tinyui_grid_track_t edges[] = {
            { TINYUI_GRID_UNIT_PX, 1 },
            { TINYUI_GRID_UNIT_PX, 32767 },
            { TINYUI_GRID_UNIT_FR, 1 },
            { TINYUI_GRID_UNIT_FR, 255 },
            { TINYUI_GRID_UNIT_CONTENT, 0 },
        };
        assert(tinyui_grid_set_columns(screen, edges, 5) == TINYUI_OK);
        assert(ld_window->gridColDsc[0] == 1);
        assert(ld_window->gridColDsc[1] == 32767);
        assert(ld_window->gridColDsc[2] == LD_GRID_FR(1));
        assert(ld_window->gridColDsc[3] == LD_GRID_FR(255));
        assert(ld_window->gridColDsc[4] == LD_GRID_CONTENT);
        assert(ld_window->gridColDsc[5] == LD_GRID_TEMPLATE_LAST);
    }

    assert(tinyui_grid_set_gap(screen, -1, 0) == TINYUI_ERROR_OUT_OF_RANGE);
    assert(tinyui_grid_set_gap(screen, 0, -1) == TINYUI_ERROR_OUT_OF_RANGE);
    assert(tinyui_grid_set_align(screen, (tinyui_align_t)99, TINYUI_ALIGN_START)
           == TINYUI_ERROR_INVALID_ARG);
    assert(tinyui_grid_set_rows(screen,
                                &(const tinyui_grid_track_t){ TINYUI_GRID_UNIT_PX, (uint16_t)-1 },
                                1) == TINYUI_ERROR_OUT_OF_RANGE);

    test_shutdown();
}

static void test_grid_cell_span_and_align_on_ld_child(void)
{
    tinyui_obj_t *screen = test_boot_screen();
    tinyui_obj_t *child = tinyui_button_create(screen);
    ldBase_t *ld_child = ld_base_of(child);
    const tinyui_grid_track_t cols[] = {
        { TINYUI_GRID_UNIT_PX, 80 },
        { TINYUI_GRID_UNIT_FR, 1 },
        { TINYUI_GRID_UNIT_CONTENT, 0 },
    };
    const tinyui_grid_track_t rows[] = {
        { TINYUI_GRID_UNIT_PX, 24 },
        { TINYUI_GRID_UNIT_FR, 1 },
        { TINYUI_GRID_UNIT_PX, 16 },
    };

    assert(tinyui_grid_set_columns(screen, cols, 3) == TINYUI_OK);
    assert(tinyui_grid_set_rows(screen, rows, 3) == TINYUI_OK);
    assert(tinyui_obj_set_grid_cell(child,
                                    1, 1, 2, 3,
                                    TINYUI_ALIGN_CENTER,
                                    TINYUI_ALIGN_END) == TINYUI_OK);
    assert(ld_child->gridColPos == 1);
    assert(ld_child->gridRowPos == 1);
    assert(ld_child->gridColSpan == 2);
    assert(ld_child->gridRowSpan == 3);
    assert(ld_child->gridCellXAlign == ldGridAlignCenter);
    assert(ld_child->gridCellYAlign == ldGridAlignEnd);
    assert(ld_child->isGridCellSet == true);

    assert(tinyui_obj_set_grid_cell(child,
                                    0, 0, 0, 1,
                                    TINYUI_ALIGN_START,
                                    TINYUI_ALIGN_START) != TINYUI_OK);
    assert(ld_child->gridColSpan == 2);

    test_shutdown();
}

static void test_layout_facades_are_direct_ld_callers(void)
{
    /* tinyui_test_repo_path_from_file uses a shared static buffer; copy both
     * resolved paths before any subsequent lookup/read. */
    char flex[512];
    char grid[512];

    {
        const char *resolved;

        resolved = tinyui_test_repo_path_from_file(__FILE__, "tinyui/src/layout/flex.c");
        assert(resolved != 0);
        assert(snprintf(flex, sizeof(flex), "%s", resolved) > 0);
        resolved = tinyui_test_repo_path_from_file(__FILE__, "tinyui/src/layout/grid.c");
        assert(resolved != 0);
        assert(snprintf(grid, sizeof(grid), "%s", resolved) > 0);
    }

    assert(tinyui_test_source_contains(flex, "ldWindowSetFlexFlow"));
    assert(tinyui_test_source_contains(flex, "ldWindowSetFlexAlign"));
    assert(tinyui_test_source_contains(flex, "ldWindowSetFlexGap"));
    assert(tinyui_test_source_contains(grid, "ldWindowSetGridDscArray"));
    assert(tinyui_test_source_contains(grid, "LD_GRID_FR"));
    assert(tinyui_test_source_contains(grid, "LD_GRID_CONTENT"));
    assert(tinyui_test_source_lacks_function_definition(grid, "tinyui_layout_solve"));
    assert(tinyui_test_source_lacks_function_definition(flex, "tinyui_layout_solve"));
}

int main(void)
{
    test_layout_facades_are_direct_ld_callers();
    test_flex_rejects_invalid_object_and_flow();
    test_flex_flow_align_gap_map_to_ld_window();
    test_flex_child_grow_minmax_new_track_ignore_layout();
    test_grid_typed_tracks_map_to_ld_window();
    test_grid_parameter_conversion_failures();
    test_grid_cell_span_and_align_on_ld_child();
    return 0;
}
