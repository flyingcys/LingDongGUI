#include "picoui/picoui.h"

static void make_ui(struct picoui_window *win)
{
    struct picoui_button_props apply_props = {
        .id = "apply",
        .text = "Apply",
        .width = 96,
        .height = 36,
    };
    struct picoui_label *title;
    struct picoui_switch *wifi;
    struct picoui_slider *brightness;
    struct picoui_button *apply;

    picoui_flex_set_flow(win, PICOUI_FLEX_FLOW_COLUMN);
    picoui_flex_set_gap(win, 12, 12);

    title = picoui_label_create(win, "title");
    wifi = picoui_switch_create(win, "wifi");
    brightness = picoui_slider_create(win, "brightness");
    apply = picoui_button_create_with_props(win, &apply_props);

    picoui_label_set_text(title, "Settings");
    picoui_switch_set_checked(wifi, 1);
    picoui_slider_set_value(brightness, 75);
    (void)apply;
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
