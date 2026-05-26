#include "picoui/picoui.h"

static void make_ui(struct picoui_window *win)
{
    struct picoui_button *a;
    struct picoui_button *b;
    struct picoui_button *c;

    picoui_flex_set_flow(win, PICOUI_FLEX_FLOW_ROW_WRAP);
    picoui_flex_set_align(win,
                          PICOUI_ALIGN_START,
                          PICOUI_ALIGN_CENTER,
                          PICOUI_ALIGN_SPACE_AROUND);
    picoui_flex_set_gap(win, 8, 12);

    a = picoui_button_create(win, "first");
    b = picoui_button_create(win, "second");
    c = picoui_button_create(win, "third");
    picoui_button_set_text(a, "One");
    picoui_button_set_text(b, "Two");
    picoui_button_set_text(c, "Three");
    picoui_widget_set_flex_grow((struct picoui_widget *)a, 1);
    picoui_widget_set_flex_grow((struct picoui_widget *)b, 1);
    picoui_widget_set_flex_new_track((struct picoui_widget *)c, 1);
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
