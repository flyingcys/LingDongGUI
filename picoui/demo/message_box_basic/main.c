#include "picoui/picoui.h"

static void make_ui(struct picoui_window *win)
{
    struct picoui_label *title = picoui_label_create(win, "title");
    struct picoui_message_box_props props = {
        .id = "message_box",
        .title = "Update",
        .message = "Apply settings?",
        .confirm_text = "OK",
    };

    picoui_label_set_text(title, "Message Box");
    (void)picoui_message_box_create_with_props((struct picoui_widget *)win, &props);
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
