#include "picoui/picoui.h"

static const int cols[] = {80, -2, 0};
static const int rows[] = {32, -2, 0};

static void make_ui(struct picoui_window *win)
{
    struct picoui_label *title;
    struct picoui_button *left;
    struct picoui_button *right;

    picoui_grid_set_columns(win, cols, 3);
    picoui_grid_set_rows(win, rows, 3);
    picoui_grid_set_gap(win, 8, 8);

    title = picoui_label_create(win, "title");
    left = picoui_button_create(win, "left");
    right = picoui_button_create(win, "right");
    picoui_label_set_text(title, "Grid");
    picoui_button_set_text(left, "A");
    picoui_button_set_text(right, "B");
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
    if (picoui_app_run(app, win) != 0) {
        picoui_app_destroy(app);
        return 1;
    }
    picoui_app_destroy(app);
    return 0;
}

int main(void)
{
    return run_demo();
}
