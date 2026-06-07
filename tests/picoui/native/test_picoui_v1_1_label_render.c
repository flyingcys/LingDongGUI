#include "picoui/picoui.h"
#include "../../../picoui/src/core/internal_v1_1.h"

#include <assert.h>
#include <string.h>

int main(void)
{
    struct picoui_screen *screen;
    struct picoui_window *root;
    struct picoui_label *label;
    const char *text = "Hello PicoUI";

    assert(picoui_init() == 0);

    screen = picoui_screen_active();
    assert(screen != 0);

    root = picoui_window_create_root(screen, "root");
    assert(root != 0);

    label = picoui_label_create(root, "label");
    assert(label != 0);

    assert(picoui_label_set_text(label, text) == 0);
    assert(picoui_timer_handler() == 0);
    assert(picoui_label_get_text(label) != 0);
    assert(strcmp(picoui_label_get_text(label), text) == 0);
    assert(label->widget.dirty == 0);

    picoui_deinit();
    return 0;
}
