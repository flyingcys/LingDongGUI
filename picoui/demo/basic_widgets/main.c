#include "picoui/picoui.h"

static void on_wifi_changed(struct picoui_widget *widget, int value, void *user_data)
{
    (void)widget;
    (void)user_data;
    (void)value;
}

static void make_ui(struct picoui_window *win)
{
    struct picoui_switch *sw = picoui_switch_create(win, "wifi");
    struct picoui_checkbox *cb = picoui_checkbox_create(win, "agree");
    struct picoui_slider *slider = picoui_slider_create(win, "volume");

    picoui_switch_set_on_toggled(sw, on_wifi_changed, 0);
    picoui_checkbox_set_on_toggled(cb, on_wifi_changed, 0);
    picoui_slider_set_on_value_changed(slider, on_wifi_changed, 0);
}
