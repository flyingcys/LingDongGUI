#include "picoui/core.h"
#include "picoui/screen.h"
#include "picoui/input.h"

#include <assert.h>
#include <stdint.h>

int main(void)
{
    struct picoui_window *root_window = (struct picoui_window *)(uintptr_t)0x1;
    struct picoui_screen *screen;
    struct picoui_indev *indev;

    assert(picoui_init() == 0);
    screen = picoui_screen_active();
    assert(screen != 0);
    assert(picoui_screen_set_root_window(screen, root_window) == 0);
    assert(picoui_screen_get_root_window(screen) == root_window);
    assert(picoui_screen_load(screen) == 0);
    indev = picoui_indev_create();
    assert(indev != 0);
    assert(picoui_timer_handler() == 0);
    picoui_deinit();
    return 0;
}
