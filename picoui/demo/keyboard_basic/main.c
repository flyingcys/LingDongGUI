#include "picoui/picoui.h"

int main(void)
{
    struct picoui_app *app;
    struct picoui_window *window;

    app = picoui_app_create();
    if (app == 0) {
        return 1;
    }

    window = picoui_window_create(app, "keyboard_root");
    if (window == 0) {
        picoui_app_destroy(app);
        return 1;
    }

    if (picoui_line_edit_create_with_props(
            window,
            &(struct picoui_line_edit_props){
                .id = "keyboard_demo_input",
                .text = "abc",
                .keyboard_binding = 1U,
                .has_keyboard_binding = 1,
                .width = 220,
                .height = 32,
            }) == 0) {
        picoui_app_destroy(app);
        return 1;
    }
    if (picoui_keyboard_create_with_props(
            window,
            &(struct picoui_keyboard_props){
                .id = "keyboard_demo_keyboard",
                .width = 320,
                .height = 160,
            }) == 0) {
        picoui_app_destroy(app);
        return 1;
    }

    if (picoui_app_run(app, window) != 0) {
        picoui_app_destroy(app);
        return 1;
    }

    picoui_app_destroy(app);
    return 0;
}
