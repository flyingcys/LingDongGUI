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
    assert(ld_a->flexGrow == 1);
    assert(ld_a->flexInNewTrack == true);
    assert(ld_b->ignoreLayout == true);

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
    test_flex_layout_setters_sync_to_real_ld_window_and_children();
    return 0;
}
