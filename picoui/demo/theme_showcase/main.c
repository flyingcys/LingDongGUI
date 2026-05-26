#include "picoui/picoui.h"

static void make_ui(struct picoui_window *win)
{
    struct picoui_label *title = picoui_label_create(win, "title");
    struct picoui_text *body = picoui_text_create(win, "body");
    struct picoui_button *accent = picoui_button_create(win, "accent");

    picoui_label_set_text(title, "Theme");
    picoui_text_set_text(body, "Accent preview");
    picoui_button_set_text(accent, "Primary");
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
    picoui_app_destroy(app);
    picoui_theme_destroy(theme);
    return 0;
}

int main(void)
{
    return run_demo();
}
