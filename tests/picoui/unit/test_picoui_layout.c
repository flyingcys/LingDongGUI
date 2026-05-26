#include "internal.h"
#include "picoui/picoui.h"

#include <assert.h>

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
    return 0;
}
