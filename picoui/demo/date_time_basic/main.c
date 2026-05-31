#include "picoui/picoui.h"

static void make_ui(struct picoui_window *win)
{
    struct picoui_label *title;
    struct picoui_date_time *date_time;
    static const int cols[] = {320, 0};
    static const int rows[] = {28, 44, 0};

    picoui_grid_set_columns(win, cols, 2);
    picoui_grid_set_rows(win, rows, 3);
    picoui_grid_set_gap(win, 16, 16);
    picoui_grid_set_align(win, PICOUI_ALIGN_CENTER, PICOUI_ALIGN_START);
    picoui_window_set_padding_group(win, 24, 24, 24, 24);

    title = picoui_label_create(win, "title");
    date_time = picoui_date_time_create((struct picoui_widget *)win, "date_time");

    picoui_label_set_text(title, "Date Time");
    picoui_date_time_set_format(date_time, "yyyy-mm-dd hh:nn:ss");
    picoui_date_time_set_date(date_time, 2026, 5, 31);
    picoui_date_time_set_time(date_time, 12, 34, 56);

    picoui_widget_set_size((struct picoui_widget *)title, 180, 28);
    picoui_widget_set_size((struct picoui_widget *)date_time, 240, 32);

    picoui_widget_set_grid_cell((struct picoui_widget *)title,
                                0, 0, 1, 1,
                                PICOUI_ALIGN_CENTER,
                                PICOUI_ALIGN_CENTER);
    picoui_widget_set_grid_cell((struct picoui_widget *)date_time,
                                0, 1, 1, 1,
                                PICOUI_ALIGN_CENTER,
                                PICOUI_ALIGN_CENTER);
}

static int run_demo(void)
{
    struct picoui_theme *theme = picoui_theme_create();
    struct picoui_app *app = picoui_app_create();
    struct picoui_window *win;

    if (theme == 0 || app == 0) {
        picoui_theme_destroy(theme);
        picoui_app_destroy(app);
        return 1;
    }

    if (picoui_app_set_theme(app, theme) != 0) {
        picoui_theme_destroy(theme);
        picoui_app_destroy(app);
        return 1;
    }

    win = picoui_window_create(app, "root");
    if (win == 0) {
        picoui_theme_destroy(theme);
        picoui_app_destroy(app);
        return 1;
    }

    make_ui(win);
    if (picoui_app_run(app, win) != 0) {
        picoui_app_destroy(app);
        picoui_theme_destroy(theme);
        return 1;
    }
    picoui_app_destroy(app);
    picoui_theme_destroy(theme);
    return 0;
}

int main(void)
{
    return run_demo();
}
