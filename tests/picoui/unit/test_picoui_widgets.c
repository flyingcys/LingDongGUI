#include "picoui/picoui.h"

#include <assert.h>

static int toggled_value = -1;
static int slider_value = -1;

static void on_toggle(struct picoui_widget *widget, int value, void *user_data)
{
    (void)widget;
    (void)user_data;
    toggled_value = value;
}

static void on_slider(struct picoui_widget *widget, int value, void *user_data)
{
    (void)widget;
    (void)user_data;
    slider_value = value;
}

int main(void)
{
    struct picoui_app *app = picoui_app_create();
    struct picoui_window *win = picoui_window_create(app, "root");
    struct picoui_switch *sw = picoui_switch_create(win, "wifi");
    struct picoui_checkbox *cb = picoui_checkbox_create(win, "agree");
    struct picoui_slider *slider = picoui_slider_create(win, "volume");

    assert(sw && cb && slider);
    assert(picoui_switch_set_on_toggled(sw, on_toggle, 0) == 0);
    assert(picoui_checkbox_set_on_toggled(cb, on_toggle, 0) == 0);
    assert(picoui_slider_set_on_value_changed(slider, on_slider, 0) == 0);

    assert(picoui_switch_set_checked(sw, 1) == 0);
    assert(picoui_checkbox_set_checked(cb, 1) == 0);
    assert(picoui_slider_set_value(slider, 42) == 0);
    assert(picoui_slider_get_value(slider) == 42);
    assert(toggled_value == 1);
    assert(slider_value == 42);

    picoui_app_destroy(app);
    return 0;
}
