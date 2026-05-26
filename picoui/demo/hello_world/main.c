#include "picoui/picoui.h"

static void make_ui(struct picoui_window *win)
{
    struct picoui_label *label = picoui_label_create(win, "title");
    struct picoui_button *button = picoui_button_create(win, "ok");

    picoui_label_set_text(label, "Hello PicoUI");
    picoui_button_set_text(button, "OK");
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
