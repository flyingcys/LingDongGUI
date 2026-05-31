#include "picoui/picoui.h"

static void on_wifi_changed(struct picoui_widget *widget, int value, void *user_data)
{
    (void)widget;
    (void)user_data;
    (void)value;
}

static void on_button_clicked(struct picoui_widget *widget, void *user_data)
{
    (void)widget;
    (void)user_data;
}

static void make_ui(struct picoui_window *win)
{
    struct picoui_switch *sw = picoui_switch_create(win, "wifi");
    struct picoui_checkbox *cb = picoui_checkbox_create(win, "agree");
    struct picoui_slider *slider = picoui_slider_create(win, "volume");
    struct picoui_button *button = picoui_button_create(win, "submit");
    struct picoui_text *text = picoui_text_create(win, "title");
    struct picoui_image *image = picoui_image_create(win, "logo");
    struct picoui_image_source *image_source = 0;
    const int cols[] = {220, 0};
    const int rows[] = {36, 30, 30, 36, 28, 64, 0};

    picoui_grid_set_columns(win, cols, 2);
    picoui_grid_set_rows(win, rows, 7);
    picoui_grid_set_gap(win, 12, 12);
    picoui_grid_set_align(win, PICOUI_ALIGN_START, PICOUI_ALIGN_START);
    picoui_window_set_padding_group(win, 16, 24, 16, 16);

    picoui_widget_set_size((struct picoui_widget *)sw, 220, 36);
    picoui_widget_set_size((struct picoui_widget *)cb, 220, 30);
    picoui_widget_set_size((struct picoui_widget *)slider, 220, 30);
    picoui_widget_set_size((struct picoui_widget *)button, 160, 36);
    picoui_widget_set_size((struct picoui_widget *)text, 220, 28);
    picoui_widget_set_size((struct picoui_widget *)image, 220, 56);

    picoui_widget_set_grid_cell((struct picoui_widget *)sw, 0, 0, 1, 1, PICOUI_ALIGN_START, PICOUI_ALIGN_START);
    picoui_widget_set_grid_cell((struct picoui_widget *)cb, 0, 1, 1, 1, PICOUI_ALIGN_START, PICOUI_ALIGN_START);
    picoui_widget_set_grid_cell((struct picoui_widget *)slider, 0, 2, 1, 1, PICOUI_ALIGN_START, PICOUI_ALIGN_START);
    picoui_widget_set_grid_cell((struct picoui_widget *)button, 0, 3, 1, 1, PICOUI_ALIGN_START, PICOUI_ALIGN_START);
    picoui_widget_set_grid_cell((struct picoui_widget *)text, 0, 4, 1, 1, PICOUI_ALIGN_START, PICOUI_ALIGN_START);
    picoui_widget_set_grid_cell((struct picoui_widget *)image, 0, 5, 1, 1, PICOUI_ALIGN_START, PICOUI_ALIGN_START);

    picoui_switch_set_checked(sw, 1);
    picoui_checkbox_set_checked(cb, 1);
    picoui_slider_set_value(slider, 28);
    picoui_switch_set_on_toggled(sw, on_wifi_changed, 0);
    picoui_checkbox_set_on_toggled(cb, on_wifi_changed, 0);
    picoui_slider_set_on_value_changed(slider, on_wifi_changed, 0);
    picoui_checkbox_set_text(cb, "Wi-Fi Enabled");
    picoui_button_set_text(button, "Submit");
    picoui_button_set_on_clicked(button, on_button_clicked, 0);
    picoui_text_set_text(text, "Basic Widgets");
    picoui_image_set_source(image, image_source);
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
