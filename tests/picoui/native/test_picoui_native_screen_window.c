#include "picoui/picoui.h"

#include <assert.h>

int main(void)
{
    struct picoui_screen *screen;
    struct picoui_window *window;

    assert(picoui_init() == 0);
    screen = picoui_screen_active();
    assert(screen != 0);
    window = picoui_window_create_root(screen, "root");
    assert(window != 0);
    assert(picoui_widget_get_parent((struct picoui_widget *)window) == 0);
    picoui_deinit();
    return 0;
}
