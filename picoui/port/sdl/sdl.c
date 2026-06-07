#include "picoui/port/sdl.h"

#include "picoui/display.h"
#include "picoui/indev.h"
#include "picoui/osal.h"
#include "picoui/tick.h"

#include <SDL.h>

static struct picoui_display *g_picoui_sdl_display;
static struct picoui_indev *g_picoui_sdl_pointer_indev;

static unsigned int picoui_port_sdl_tick_get(void *user_data)
{
    (void)user_data;
    return (unsigned int)SDL_GetTicks();
}

static void picoui_port_sdl_delay(unsigned int ms, void *user_data)
{
    (void)user_data;
    SDL_Delay((Uint32)ms);
}

int picoui_sdl_hal_init(int width, int height)
{
    struct picoui_display *display;
    struct picoui_indev *pointer_indev;

    if (width <= 0 || height <= 0) {
        return -1;
    }

    if (SDL_WasInit(SDL_INIT_VIDEO | SDL_INIT_EVENTS) == 0
        && SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) != 0) {
        return -1;
    }

    display = picoui_display_create(width, height);
    if (display == 0) {
        return -1;
    }

    if (picoui_display_set_default(display) != 0) {
        return -1;
    }

    pointer_indev = picoui_indev_create();
    if (pointer_indev == 0) {
        return -1;
    }
    if (picoui_indev_set_type(pointer_indev, PICOUI_INDEV_TYPE_POINTER) != 0) {
        return -1;
    }

    g_picoui_sdl_display = display;
    g_picoui_sdl_pointer_indev = pointer_indev;
    return 0;
}

int picoui_port_sdl_default_pointer_indev(struct picoui_indev **out_indev)
{
    if (out_indev == 0 || g_picoui_sdl_pointer_indev == 0) {
        return -1;
    }

    *out_indev = g_picoui_sdl_pointer_indev;
    return 0;
}

int picoui_port_sdl_attach(struct picoui_app *app)
{
    const struct picoui_display_config display = {
        .width = 480,
        .height = 320,
        .color_format = PICOUI_COLOR_FORMAT_RGB565,
        .buffer_height = 0,
        .user_data = NULL,
    };

    if (app == NULL) {
        return -1;
    }

    if (picoui_display_set_config(app, &display) != 0) {
        return -1;
    }
    if (picoui_tick_set_source(app, picoui_port_sdl_tick_get, NULL) != 0) {
        return -1;
    }
    if (picoui_os_set_delay_callback(app, picoui_port_sdl_delay, NULL) != 0) {
        return -1;
    }

    return 0;
}
