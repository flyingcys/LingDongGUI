#include "tinyui.h"

int main(void)
{
    tinyui_obj_t *screen;
    tinyui_obj_t *label;
    tinyui_obj_t *button;
    uint32_t next_ms = 0;

    if (tinyui_init() != TINYUI_OK) {
        return 1;
    }

    screen = tinyui_screen_create();
    if (screen == 0) {
        tinyui_deinit();
        return 1;
    }

    label = tinyui_label_create(screen);
    button = tinyui_button_create(screen);
    if (label == 0 || button == 0
        || tinyui_label_set_text(label, "Minimal TinyUI") != 0
        || tinyui_button_set_text(button, "OK") != 0
        || tinyui_screen_load(screen, TINYUI_SCREEN_TRANSITION_NONE, 0) != TINYUI_OK) {
        tinyui_deinit();
        return 1;
    }

    if (tinyui_process(&next_ms) != TINYUI_OK) {
        tinyui_deinit();
        return 1;
    }

    tinyui_deinit();
    return 0;
}
