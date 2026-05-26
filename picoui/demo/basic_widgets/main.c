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

    picoui_switch_set_on_toggled(sw, on_wifi_changed, 0);
    picoui_checkbox_set_on_toggled(cb, on_wifi_changed, 0);
    picoui_slider_set_on_value_changed(slider, on_wifi_changed, 0);
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
    picoui_app_destroy(app);
    return 0;
}

int main(void)
{
    return run_demo();
}
