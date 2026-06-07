#include "picoui/core.h"
#include "picoui/input.h"
#include "picoui/display_v1_1.h"
#include "picoui/port/sdl_v1_1.h"

#include <assert.h>

int main(void)
{
    struct picoui_indev *indev = 0;
    struct picoui_display *display;
    int width = 0;
    int height = 0;

    assert(picoui_init() == 0);
    assert(picoui_sdl_hal_init(320, 240) == 0);
    display = picoui_display_get_default();
    assert(display != 0);
    assert(picoui_display_get_size(display, &width, &height) == 0);
    assert(width == 320);
    assert(height == 240);
    assert(picoui_port_sdl_default_pointer_indev(&indev) == 0);
    assert(indev != 0);
    picoui_deinit();
    return 0;
}
