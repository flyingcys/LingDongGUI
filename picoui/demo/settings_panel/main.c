#include "picoui/picoui.h"

static void make_ui(struct picoui_window *win)
{
    struct picoui_label *title;
    struct picoui_switch *wifi;
    struct picoui_slider *brightness;
    struct picoui_button *apply;
    static const int cols[] = {-2, -1, 0};
    static const int rows[] = {32, 36, 40, 0};

    picoui_grid_set_columns(win, cols, 3);
    picoui_grid_set_rows(win, rows, 4);
    picoui_grid_set_gap(win, 12, 12);
    picoui_grid_set_align(win, PICOUI_ALIGN_STRETCH, PICOUI_ALIGN_START);

    title = picoui_label_create(win, "title");
    wifi = picoui_switch_create(win, "wifi");
    brightness = picoui_slider_create(win, "brightness");
    apply = picoui_button_create(win, "apply");

    picoui_label_set_text(title, "Settings");
    picoui_switch_set_checked(wifi, 1);
    picoui_slider_set_value(brightness, 75);
    picoui_button_set_text(apply, "Apply");
    picoui_widget_set_grid_cell((struct picoui_widget *)title,
                                0, 0, 2, 1,
                                PICOUI_ALIGN_START,
                                PICOUI_ALIGN_CENTER);
    picoui_widget_set_grid_cell((struct picoui_widget *)wifi,
                                0, 1, 2, 1,
                                PICOUI_ALIGN_STRETCH,
                                PICOUI_ALIGN_CENTER);
    picoui_widget_set_grid_cell((struct picoui_widget *)brightness,
                                0, 2, 2, 1,
                                PICOUI_ALIGN_STRETCH,
                                PICOUI_ALIGN_CENTER);
    picoui_widget_set_grid_cell((struct picoui_widget *)apply,
                                1, 3, 1, 1,
                                PICOUI_ALIGN_END,
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
