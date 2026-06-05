#include "picoui/picoui.h"

#include <assert.h>

static void flush_cb(const struct picoui_area *area, const void *pixels, void *user_data)
{
    (void)area;
    (void)pixels;
    int *called = (int *)user_data;
    *called = 1;
}

static void indev_read_cb(struct picoui_indev *indev, struct picoui_indev_data *data, void *user_data)
{
    (void)indev;
    (void)user_data;
    data->pointer_x = 12;
    data->pointer_y = 34;
    data->pressed = 1;
}

int main(void)
{
    int called = 0;
    struct picoui_display *display;
    struct picoui_indev *indev;

    assert(picoui_init() == 0);
    display = picoui_display_create(320, 480);
    assert(display != 0);
    assert(picoui_display_set_flush_cb(display, flush_cb, &called) == 0);
    assert(picoui_display_set_default(display) == 0);
    assert(picoui_display_get_default() == display);

    indev = picoui_indev_create();
    assert(indev != 0);
    assert(picoui_indev_set_type(indev, PICOUI_INDEV_TYPE_POINTER) == 0);
    assert(picoui_indev_set_read_cb(indev, indev_read_cb, 0) == 0);

    picoui_deinit();
    return 0;
}
