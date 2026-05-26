#include "picoui/picoui.h"

static void make_ui(struct picoui_app *app)
{
    struct picoui_window *win = picoui_window_create(app, "root");
    struct picoui_label *label = picoui_label_create(win, "title");
    struct picoui_button *button = picoui_button_create(win, "ok");

    picoui_label_set_text(label, "Hello PicoUI");
    picoui_button_set_text(button, "OK");
}
