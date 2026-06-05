#include "picoui/display.h"

#include <stdlib.h>

struct picoui_display {
    int width;
    int height;
    picoui_display_flush_cb_t flush_cb;
    void *flush_user_data;
};

static struct picoui_display *g_default_display;

struct picoui_display *picoui_display_create(int width, int height)
{
    struct picoui_display *display;

    if (width <= 0 || height <= 0) {
        return 0;
    }

    display = (struct picoui_display *)calloc(1, sizeof(*display));
    if (display == 0) {
        return 0;
    }

    display->width = width;
    display->height = height;
    return display;
}

int picoui_display_set_default(struct picoui_display *display)
{
    if (display == 0) {
        return -1;
    }

    g_default_display = display;
    return 0;
}

struct picoui_display *picoui_display_get_default(void)
{
    return g_default_display;
}

int picoui_display_get_size(const struct picoui_display *display, int *width, int *height)
{
    if (display == 0 || width == 0 || height == 0) {
        return -1;
    }

    *width = display->width;
    *height = display->height;
    return 0;
}

int picoui_display_set_flush_cb(struct picoui_display *display,
                                picoui_display_flush_cb_t callback,
                                void *user_data)
{
    if (display == 0 || callback == 0) {
        return -1;
    }

    display->flush_cb = callback;
    display->flush_user_data = user_data;
    return 0;
}
