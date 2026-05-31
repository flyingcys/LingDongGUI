#include "picoui/picoui.h"

static void make_ui(struct picoui_window *win)
{
    struct picoui_label *title;
    struct picoui_combo_box *combo_box;
    static const int cols[] = {240, 0};
    static const int rows[] = {28, 36, 0};

    picoui_grid_set_columns(win, cols, 2);
    picoui_grid_set_rows(win, rows, 3);
    picoui_grid_set_gap(win, 12, 12);
    picoui_grid_set_align(win, PICOUI_ALIGN_START, PICOUI_ALIGN_START);
    picoui_window_set_padding_group(win, 24, 24, 24, 24);

    title = picoui_label_create(win, "title");
    combo_box = picoui_combo_box_create_with_props(
        win,
        &(struct picoui_combo_box_props){
            .id = "combo_box",
            .width = 220,
            .height = 32,
        });

    if (title != 0) {
        picoui_label_set_text(title, "Combo Box");
        picoui_widget_set_grid_cell((struct picoui_widget *)title,
                                    0, 0, 1, 1,
                                    PICOUI_ALIGN_START,
                                    PICOUI_ALIGN_CENTER);
    }

    if (combo_box != 0) {
        picoui_combo_box_add_item(combo_box, "wifi", "Wi-Fi");
        picoui_combo_box_add_item(combo_box, "bluetooth", "Bluetooth");
        picoui_combo_box_add_item(combo_box, "display", "Display");
        picoui_combo_box_set_selected_index(combo_box, 1);
        picoui_widget_set_grid_cell((struct picoui_widget *)combo_box,
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
    if (picoui_app_run(app, win) != 0) {
        picoui_app_destroy(app);
        return 1;
    }

    picoui_app_destroy(app);
    return 0;
}
