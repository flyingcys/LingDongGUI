#include "tinyui.h"

int main(void)
{
    tinyui_obj_t *screen;
    struct tinyui_window *window;
    struct tinyui_label *label;
    struct tinyui_button *button;

    if (tinyui_init() != 0) {
        return 1;
    }

    screen = tinyui_screen_create();
    if (screen == 0) {
        tinyui_deinit();
        return 1;
    }

    window = (struct tinyui_window *)screen;
    label = tinyui_label_create(window, "title");
    button = tinyui_button_create(window, "action");
    if (label == 0 || button == 0
        || tinyui_label_set_text(label, "Minimal TinyUI") != 0
        || tinyui_button_set_text(button, "OK") != 0
        || tinyui_screen_load(screen) != 0) {
        tinyui_deinit();
        return 1;
    }

    (void)tinyui_timer_handler();
    tinyui_deinit();
    return 0;
}
