#include "picoui/picoui.h"

#include <assert.h>

int picoui_native_render_bind_root(struct picoui_screen *screen, struct picoui_window *root_window);

int main(void)
{
    struct picoui_display *display;
    struct picoui_screen *screen;
    struct picoui_window *window;
    struct picoui_label *label;
    struct picoui_button *button;
    const char *button_text = 0;
    int rc;

    assert(picoui_init() == 0);

    display = picoui_display_create(320, 480);
    assert(display != 0);
    assert(picoui_display_set_default(display) == 0);

    screen = picoui_screen_active();
    assert(screen != 0);

    window = picoui_window_create_root(screen, "root");
    assert(window != 0);

    label = picoui_label_create(window, "status_label");
    assert(label != 0);
    assert(picoui_label_set_text(label, "native render smoke") == 0);

    button = picoui_button_create(window, "confirm_button");
    assert(button != 0);
    assert(picoui_button_set_text(button, "OK") == 0);

    assert(picoui_widget_get_child_count((const struct picoui_widget *)window) == 2);
    assert(picoui_widget_get_type((const struct picoui_widget *)label) == PICOUI_WIDGET_TYPE_LABEL);
    assert(picoui_widget_get_type((const struct picoui_widget *)button) == PICOUI_WIDGET_TYPE_BUTTON);
    assert(picoui_label_get_text(label) != 0);
    assert(picoui_button_get_text(button, &button_text) == 0);
    assert(button_text != 0);
    assert(picoui_native_render_bind_root(screen, window) == 0);

    rc = picoui_timer_handler();
    assert(rc == 0 || rc == 1);

    picoui_deinit();
    return 0;
}
