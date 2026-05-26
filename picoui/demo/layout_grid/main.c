#include "picoui/picoui.h"

static const int cols[] = {80, -2, 0};
static const int rows[] = {32, -2, 0};

static void make_ui(struct picoui_window *win)
{
    picoui_grid_set_columns(win, cols, 3);
    picoui_grid_set_rows(win, rows, 3);
    picoui_grid_set_gap(win, 8, 8);
}

static int run_demo(void)
{
    struct picoui_app *app = picoui_app_create();
    struct picoui_window *win;

    if (app == 0) {
        return 1;
    }

    win = picoui_window_create(app, "root");
    if (win == 0) {
        picoui_app_destroy(app);
        return 1;
    }

    make_ui(win);
    picoui_app_destroy(app);
    return 0;
}

int main(void)
{
    return run_demo();
}
