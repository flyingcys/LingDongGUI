#include "picoui/picoui.h"
#include <assert.h>

int main(void)
{
    struct picoui_indev *indev;
    struct picoui_screen *screen;

    assert(picoui_init() == 0);
    indev = picoui_indev_create();
    assert(indev != 0);
    screen = picoui_screen_active();
    assert(screen != 0);
    assert(picoui_screen_load(screen) == 0);
    assert(picoui_timer_handler() == 0);
    picoui_deinit();
    return 0;
}
