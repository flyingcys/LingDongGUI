#include "picoui/picoui.h"

#include <assert.h>

int picoui_sdl_hal_init(int width, int height);
int picoui_port_sdl_default_pointer_indev(struct picoui_indev **out_indev);
int picoui_native_indev_get_type(const struct picoui_indev *indev, enum picoui_indev_type *out_type);

static void test_sdl_hal_init_rejects_invalid_size(void)
{
    assert(picoui_init() == 0);
    assert(picoui_sdl_hal_init(0, 480) == -1);
    assert(picoui_sdl_hal_init(320, 0) == -1);
    picoui_deinit();
}

static void test_sdl_hal_init_registers_default_display_and_pointer_indev(void)
{
    struct picoui_display *display;
    struct picoui_indev *indev = 0;
    enum picoui_indev_type indev_type = PICOUI_INDEV_TYPE_NONE;
    int width = 0;
    int height = 0;

    assert(picoui_init() == 0);
    assert(picoui_sdl_hal_init(320, 480) == 0);

    display = picoui_display_get_default();
    assert(display != 0);
    assert(picoui_display_get_size(display, &width, &height) == 0);
    assert(width == 320);
    assert(height == 480);

    assert(picoui_port_sdl_default_pointer_indev(&indev) == 0);
    assert(indev != 0);
    assert(picoui_native_indev_get_type(indev, &indev_type) == 0);
    assert(indev_type == PICOUI_INDEV_TYPE_POINTER);

    picoui_deinit();
}

int main(void)
{
    test_sdl_hal_init_rejects_invalid_size();
    test_sdl_hal_init_registers_default_display_and_pointer_indev();
    return 0;
}
