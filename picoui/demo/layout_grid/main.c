#include "picoui/picoui.h"

static const int cols[] = {80, -2, 0};
static const int rows[] = {32, -2, 0};

static void make_ui(struct picoui_window *win)
{
    picoui_grid_set_columns(win, cols, 3);
    picoui_grid_set_rows(win, rows, 3);
    picoui_grid_set_gap(win, 8, 8);
}
