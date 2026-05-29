#include "picoui/picoui.h"

static void make_ui(struct picoui_window *win)
{
    struct picoui_label *title;
    struct picoui_list *list;
    static const int cols[] = {220, 0};
    static const int rows[] = {32, 104, 0};

    picoui_grid_set_columns(win, cols, 2);
    picoui_grid_set_rows(win, rows, 3);
    picoui_grid_set_gap(win, 8, 8);
    picoui_grid_set_align(win, PICOUI_ALIGN_START, PICOUI_ALIGN_START);

    title = picoui_label_create(win, "title");
    list = picoui_list_create(win, "list");

    if (title != 0) {
        picoui_label_set_text(title, "List");
        picoui_widget_set_grid_cell((struct picoui_widget *)title,
                                    0, 0, 1, 1,
                                    PICOUI_ALIGN_START,
                                    PICOUI_ALIGN_CENTER);
    }

    if (list != 0) {
        picoui_list_add_item(list, "item_wifi", "Wi-Fi");
        picoui_list_add_item(list, "item_bluetooth", "Bluetooth");
        picoui_list_add_item(list, "item_display", "Display");
        picoui_list_set_selected_index(list, 0);
        picoui_widget_set_grid_cell((struct picoui_widget *)list,
                                    0, 1, 1, 1,
                                    PICOUI_ALIGN_STRETCH,
                                    PICOUI_ALIGN_START);
    }
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
