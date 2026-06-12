#include "port/sdl.h"

#include "display.h"
#include "osal.h"
#include "tick.h"

#include <SDL.h>

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
