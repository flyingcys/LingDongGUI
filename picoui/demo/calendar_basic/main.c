#include "picoui/picoui.h"

static void make_ui(struct picoui_window *win)
{
    struct picoui_label *title;
    struct picoui_calendar *calendar;
    static const int cols[] = {320, 0};
    static const int rows[] = {28, 196, 0};

    picoui_grid_set_columns(win, cols, 2);
    picoui_grid_set_rows(win, rows, 3);
    picoui_grid_set_gap(win, 12, 12);
    picoui_grid_set_align(win, PICOUI_ALIGN_START, PICOUI_ALIGN_START);
    picoui_window_set_padding_group(win, 24, 24, 24, 24);

    title = picoui_label_create(win, "title");
    if (title != 0) {
        picoui_label_set_text(title, "Calendar");
        picoui_widget_set_size((struct picoui_widget *)title, 220, 28);
        picoui_widget_set_grid_cell((struct picoui_widget *)title,
                                    0, 0, 1, 1,
                                    PICOUI_ALIGN_START,
                                    PICOUI_ALIGN_CENTER);
    }

    calendar = picoui_calendar_create_with_props(
        win,
        &(struct picoui_calendar_props){
            .id = "calendar",
            .year = 2026,
            .month = 6,
            .day = 15,
            .width = 280,
            .height = 180,
            .show_header = 1,
            .header_format = "yyyy/mm/dd",
        });
    if (calendar != 0) {
        picoui_widget_set_grid_cell((struct picoui_widget *)calendar,
                                    0, 1, 1, 1,
                                    PICOUI_ALIGN_START,
                                    PICOUI_ALIGN_CENTER);
    }
}

int main(void)
{
    struct picoui_app *app;
    struct picoui_window *win;

    app = picoui_app_create();
    if (app == 0) {
        return 1;
    }

    win = picoui_window_create(app, "root");
    if (win == 0) {
        picoui_app_destroy(app);
        return 1;
    }

    make_ui(win);
    return picoui_app_run(app, win);
}
