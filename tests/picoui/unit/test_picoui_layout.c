#include "internal.h"
#include "picoui/picoui.h"
#include "ldBase.h"
#include "ldWindow.h"

#include <assert.h>
#include <stdbool.h>

static void test_grid_layout_setters_sync_to_real_ld_window_and_children(void)
{
    struct picoui_app *app = picoui_app_create();
    struct picoui_window *win = picoui_window_create(app, "root");
    struct picoui_button *a = picoui_button_create(win, "a");
    struct picoui_button *b = picoui_button_create(win, "b");
    const struct picoui_backend_widget *win_backend = win->widget.backend_widget;
    const struct picoui_backend_widget *b_backend = b->widget.backend_widget;
    const ldWindow_t *ld_window = (const ldWindow_t *)win_backend->ld_widget;
    const ldBase_t *ld_b = (const ldBase_t *)b_backend->ld_widget;

    assert(picoui_grid_set_columns(win, (int[]){80, -1, 0}, 3) == 0);
    assert(picoui_grid_set_rows(win, (int[]){24, -1, 0}, 3) == 0);
    assert(picoui_grid_set_gap(win, 6, 10) == 0);
    assert(picoui_grid_set_align(win, PICOUI_ALIGN_END, PICOUI_ALIGN_SPACE_AROUND) == 0);
    assert(picoui_widget_set_padding((struct picoui_widget *)win, 9) == 0);
    assert(picoui_widget_set_grid_cell((struct picoui_widget *)a,
                                       0, 0, 1, 1,
                                       PICOUI_ALIGN_START,
                                       PICOUI_ALIGN_STRETCH) == 0);
    assert(picoui_widget_set_grid_cell((struct picoui_widget *)b,
                                       1, 1, 2, 3,
                                       PICOUI_ALIGN_CENTER,
                                       PICOUI_ALIGN_END) == 0);

    assert(ld_window->layoutTpye == layoutGrid);
    assert(ld_window->gridColDsc != 0);
    assert(ld_window->gridRowDsc != 0);
    assert(ld_window->gridColDsc[0] == 80);
    assert(ld_window->gridColDsc[1] < 0);
    assert(ld_window->gridColDsc[1] != LD_GRID_TEMPLATE_LAST);
    assert(ld_window->gridColDsc[2] == LD_GRID_TEMPLATE_LAST);
    assert(ld_window->gridRowDsc[0] == 24);
    assert(ld_window->gridRowDsc[1] < 0);
    assert(ld_window->gridRowDsc[1] != LD_GRID_TEMPLATE_LAST);
    assert(ld_window->gridRowDsc[2] == LD_GRID_TEMPLATE_LAST);
    assert(ld_window->gridColumnGap == 10);
    assert(ld_window->gridRowGap == 6);
    assert(win->widget.padding == 9);
    assert(win_backend->window_layout.padding == 9);
    assert(ld_window->gridPadding.left == 9);
    assert(ld_window->gridPadding.top == 9);
    assert(ld_window->gridPadding.right == 9);
    assert(ld_window->gridPadding.bottom == 9);
    assert(ld_window->gridColAlign == ldGridAlignEnd);
    assert(ld_window->gridRowAlign == ldGridAlignSpaceAround);
    assert(ld_b->gridColPos == 1);
    assert(ld_b->gridRowPos == 1);
    assert(ld_b->gridColSpan == 2);
    assert(ld_b->gridRowSpan == 3);
    assert(ld_b->gridCellXAlign == ldGridAlignCenter);
    assert(ld_b->gridCellYAlign == ldGridAlignEnd);
    assert(ld_b->isGridCellSet == true);

    picoui_app_destroy(app);
}

static void test_grid_layout_rejects_invalid_gap_and_cell_span(void)
{
    struct picoui_app *app = picoui_app_create();
    struct picoui_window *win = picoui_window_create(app, "root");
    struct picoui_button *a = picoui_button_create(win, "a");
    const struct picoui_backend_widget *win_backend = win->widget.backend_widget;
    const struct picoui_backend_widget *a_backend = a->widget.backend_widget;
    const ldWindow_t *ld_window = (const ldWindow_t *)win_backend->ld_widget;
    const ldBase_t *ld_a = (const ldBase_t *)a_backend->ld_widget;

    assert(picoui_grid_set_gap(win, 4, 5) == 0);
    assert(picoui_widget_set_padding((struct picoui_widget *)win, 3) == 0);
    assert(picoui_grid_set_gap(win, -1, 5) == -1);
    assert(picoui_grid_set_gap(win, 4, -1) == -1);
    assert(picoui_widget_set_padding((struct picoui_widget *)win, -1) == -1);
    assert(win->widget.padding == 3);
    assert(win_backend->window_layout.padding == 3);
    assert(ld_window->gridRowGap == 4);
    assert(ld_window->gridColumnGap == 5);
    assert(ld_window->gridPadding.left == 3);
    assert(ld_window->gridPadding.top == 3);
    assert(ld_window->gridPadding.right == 3);
    assert(ld_window->gridPadding.bottom == 3);

    assert(picoui_widget_set_grid_cell((struct picoui_widget *)a,
                                       0, 0, 2, 1,
                                       PICOUI_ALIGN_STRETCH,
                                       PICOUI_ALIGN_CENTER) == 0);
    assert(picoui_widget_set_grid_cell((struct picoui_widget *)a,
                                       0, 0, 0, 1,
                                       PICOUI_ALIGN_START,
                                       PICOUI_ALIGN_START) == -1);
    assert(picoui_widget_set_grid_cell((struct picoui_widget *)a,
                                       0, 0, 1, 0,
                                       PICOUI_ALIGN_START,
                                       PICOUI_ALIGN_START) == -1);
    assert(a->widget.grid_col_span == 2);
    assert(a->widget.grid_row_span == 1);
    assert(a->widget.grid_x_align == PICOUI_ALIGN_STRETCH);
    assert(a->widget.grid_y_align == PICOUI_ALIGN_CENTER);
    assert(ld_a->gridColSpan == 2);
    assert(ld_a->gridRowSpan == 1);
    assert(ld_a->gridCellXAlign == ldGridAlignStretch);
    assert(ld_a->gridCellYAlign == ldGridAlignCenter);

    picoui_app_destroy(app);
}

static void test_flex_layout_setters_sync_to_real_ld_window_and_children(void)
{
    struct picoui_app *app = picoui_app_create();
    struct picoui_window *win = picoui_window_create(app, "root");
    struct picoui_button *a = picoui_button_create(win, "a");
    struct picoui_button *b = picoui_button_create(win, "b");
    const struct picoui_backend_widget *win_backend = win->widget.backend_widget;
    const struct picoui_backend_widget *a_backend = a->widget.backend_widget;
    const struct picoui_backend_widget *b_backend = b->widget.backend_widget;
    const ldWindow_t *ld_window = (const ldWindow_t *)win_backend->ld_widget;
    const ldBase_t *ld_a = (const ldBase_t *)a_backend->ld_widget;
    const ldBase_t *ld_b = (const ldBase_t *)b_backend->ld_widget;

    assert(picoui_flex_set_flow(win, PICOUI_FLEX_FLOW_ROW_WRAP) == 0);
    assert(picoui_flex_set_align(win,
                                 PICOUI_ALIGN_START,
                                 PICOUI_ALIGN_CENTER,
                                 PICOUI_ALIGN_SPACE_BETWEEN) == 0);
    assert(picoui_flex_set_gap(win, 8, 12) == 0);
    assert(picoui_widget_set_padding((struct picoui_widget *)win, 7) == 0);
    assert(picoui_flex_set_gap(win, -1, 12) == -1);
    assert(picoui_flex_set_gap(win, 8, -1) == -1);
    assert(picoui_widget_set_padding((struct picoui_widget *)win, -1) == -1);
    assert(picoui_widget_set_flex_grow((struct picoui_widget *)a, 1) == 0);
    assert(picoui_widget_set_flex_new_track((struct picoui_widget *)a, 1) == 0);
    assert(picoui_widget_set_ignore_layout((struct picoui_widget *)b, 1) == 0);

    assert(ld_window->layoutTpye == layoutFlex);
    assert(ld_window->flexFlow == ldFlexFlowRowWrap);
    assert(ld_window->flexMainAlign == ldFlexMainAlignStart);
    assert(ld_window->flexCrossAlign == ldFlexCrossAlignCenter);
    assert(ld_window->flexTrackAlign == ldFlexTrackAlignSpaceBetween);
    assert(ld_window->flexItemGap == 8);
    assert(ld_window->flexTrackGap == 12);
    assert(win->widget.padding == 7);
    assert(win_backend->window_layout.padding == 7);
    assert(ld_window->flexPadding.left == 7);
    assert(ld_window->flexPadding.top == 7);
    assert(ld_window->flexPadding.right == 7);
    assert(ld_window->flexPadding.bottom == 7);
    assert(ld_a->flexGrow == 1);
    assert(ld_a->flexInNewTrack == true);
    assert(ld_b->ignoreLayout == true);

    picoui_app_destroy(app);
}

static void test_flex_layout_relayout_uses_ld_window_without_cursor_override(void)
{
    struct picoui_app *app = picoui_app_create();
    struct picoui_window *win = picoui_window_create(app, "root");
    struct picoui_button *a = picoui_button_create(win, "a");
    struct picoui_button *b = picoui_button_create(win, "b");
    const struct picoui_backend_widget *win_backend = win->widget.backend_widget;
    const struct picoui_backend_widget *a_backend = a->widget.backend_widget;
    const struct picoui_backend_widget *b_backend = b->widget.backend_widget;
    ldWindow_t *ld_window = (ldWindow_t *)win_backend->ld_widget;
    const ldBase_t *ld_a = (const ldBase_t *)a_backend->ld_widget;
    const ldBase_t *ld_b = (const ldBase_t *)b_backend->ld_widget;

    assert(picoui_flex_set_flow(win, PICOUI_FLEX_FLOW_ROW) == 0);
    assert(picoui_flex_set_gap(win, 12, 0) == 0);
    assert(picoui_widget_set_size((struct picoui_widget *)a, 50, 20) == 0);
    assert(picoui_widget_set_size((struct picoui_widget *)b, 60, 20) == 0);
    assert(ld_window->isLayoutUpdate == true);

    ldWindow_on_frame_start(NULL, ld_window);

    assert(ld_window->isLayoutUpdate == false);
    assert(ld_a->use_as__arm_2d_control_node_t.tRegion.tLocation.iX == 0);
    assert(ld_a->use_as__arm_2d_control_node_t.tRegion.tLocation.iY == 0);
    assert(ld_b->use_as__arm_2d_control_node_t.tRegion.tLocation.iX == 62);
    assert(ld_b->use_as__arm_2d_control_node_t.tRegion.tLocation.iY == 0);

    assert(picoui_flex_set_gap(win, 20, 0) == 0);
    assert(ld_window->isLayoutUpdate == true);
    ldWindow_on_frame_start(NULL, ld_window);

    assert(ld_window->isLayoutUpdate == false);
    assert(ld_b->use_as__arm_2d_control_node_t.tRegion.tLocation.iX == 70);

    picoui_app_destroy(app);
}

static void test_window_padding_survives_layout_type_switches(void)
{
    struct picoui_app *app = picoui_app_create();
    struct picoui_window *win = picoui_window_create(app, "root");
    const struct picoui_backend_widget *win_backend = win->widget.backend_widget;
    ldWindow_t *ld_window = (ldWindow_t *)win_backend->ld_widget;

    assert(picoui_widget_set_padding((struct picoui_widget *)win, 9) == 0);
    assert(picoui_grid_set_columns(win, (int[]){40, 0}, 2) == 0);
    assert(ld_window->layoutTpye == layoutGrid);
    assert(ld_window->gridPadding.left == 9);
    assert(ld_window->gridPadding.top == 9);
    assert(ld_window->gridPadding.right == 9);
    assert(ld_window->gridPadding.bottom == 9);

    assert(picoui_widget_set_padding((struct picoui_widget *)win, 13) == 0);
    assert(picoui_flex_set_flow(win, PICOUI_FLEX_FLOW_ROW) == 0);
    assert(ld_window->layoutTpye == layoutFlex);
    assert(ld_window->flexPadding.left == 13);
    assert(ld_window->flexPadding.top == 13);
    assert(ld_window->flexPadding.right == 13);
    assert(ld_window->flexPadding.bottom == 13);

    picoui_app_destroy(app);
}

int main(void)
{
    struct picoui_app *app = picoui_app_create();
    struct picoui_window *win = picoui_window_create(app, "root");
    struct picoui_button *a = picoui_button_create(win, "a");
    struct picoui_button *b = picoui_button_create(win, "b");
    const struct picoui_backend_widget *win_backend = win->widget.backend_widget;
    const struct picoui_backend_widget *a_backend = a->widget.backend_widget;
    const struct picoui_backend_widget *b_backend = b->widget.backend_widget;

    assert(picoui_flex_set_flow(win, PICOUI_FLEX_FLOW_ROW_WRAP) == 0);
    assert(picoui_flex_set_align(win,
                                 PICOUI_ALIGN_START,
                                 PICOUI_ALIGN_CENTER,
                                 PICOUI_ALIGN_SPACE_BETWEEN) == 0);
    assert(picoui_flex_set_gap(win, 8, 12) == 0);
    assert(picoui_widget_set_flex_grow((struct picoui_widget *)a, 1) == 0);
    assert(picoui_widget_set_flex_new_track((struct picoui_widget *)a, 1) == 0);
    assert(a->widget.flex_new_track == 1);
    assert(a_backend->child_layout.flex_new_track == 1);
    assert(picoui_widget_set_ignore_layout((struct picoui_widget *)b, 1) == 0);
    assert(b->widget.ignore_layout == 1);
    assert(b_backend->child_layout.ignore_layout == 1);

    assert(picoui_grid_set_columns(win, (int[]){80, -1, 0}, 3) == 0);
    assert(picoui_grid_set_rows(win, (int[]){24, -1, 0}, 3) == 0);
    assert(picoui_grid_set_align(win, PICOUI_ALIGN_END, PICOUI_ALIGN_SPACE_AROUND) == 0);
    assert(win->grid_col_align == PICOUI_ALIGN_END);
    assert(win->grid_row_align == PICOUI_ALIGN_SPACE_AROUND);
    assert(win_backend->window_layout.grid_col_align == PICOUI_ALIGN_END);
    assert(win_backend->window_layout.grid_row_align == PICOUI_ALIGN_SPACE_AROUND);
    assert(picoui_widget_set_grid_cell((struct picoui_widget *)b,
                                       1, 0, 1, 1,
                                       PICOUI_ALIGN_CENTER,
                                       PICOUI_ALIGN_CENTER) == 0);

    picoui_app_destroy(app);
    test_grid_layout_setters_sync_to_real_ld_window_and_children();
    test_grid_layout_rejects_invalid_gap_and_cell_span();
    test_flex_layout_setters_sync_to_real_ld_window_and_children();
    test_flex_layout_relayout_uses_ld_window_without_cursor_override();
    test_window_padding_survives_layout_type_switches();
    return 0;
}
