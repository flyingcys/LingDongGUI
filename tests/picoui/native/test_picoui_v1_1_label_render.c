#include "picoui/picoui.h"

#include <assert.h>
#include <string.h>

int main(void)
{
    struct picoui_display *display;
    struct picoui_screen *screen;
    struct picoui_window *root;
    struct picoui_label *label;
    int timer_rc;

    assert(picoui_init() == 0);

    display = picoui_display_create(320, 480);
    assert(display != 0);
    assert(picoui_display_set_default(display) == 0);

    screen = picoui_screen_active();
    assert(screen != 0);

    root = picoui_window_create_root(screen, "root");
    assert(root != 0);

    label = picoui_label_create(root, "title");
    assert(label != 0);

    assert(picoui_label_set_text(label, "Hello PicoUI") == 0);
    assert(picoui_widget_set_text((struct picoui_widget *)label, "Hello Local Truth") == 0);
    assert(picoui_label_get_text(label) != 0);
    assert(strcmp(picoui_label_get_text(label), "Hello Local Truth") == 0);

    assert(picoui_screen_load(screen) == 0);
    timer_rc = picoui_timer_handler();
    assert(timer_rc == 0 || timer_rc == 1);
    assert(picoui_label_get_text(label) != 0);
    assert(strcmp(picoui_label_get_text(label), "Hello Local Truth") == 0);

    picoui_deinit();
    return 0;
}
