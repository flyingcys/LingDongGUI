/*
 * M3 Task 10 dedicated evidence runner for theme_layout_resource.
 */

#include "v23_theme_layout_resource/v23_theme_layout_resource.h"
#include "tinyui.h"
#include "display/display.h"
#include "tinyui_sdl.h"
#include "internal/runtime_internal_legacy_api.h"

#include <SDL.h>
#include <stdio.h>

int tinyui_display_set_default_config(const struct tinyui_display_config *config);

int main(int argc, char **argv)
{
    unsigned int previous_ticks;
    struct tinyui_display_config display_config = {0};

    (void)argc;
    (void)argv;

    if (tinyui_init() != 0) {
        return 1;
    }

    display_config.width = 480;
    display_config.height = 320;
    display_config.color_format = TINYUI_COLOR_FORMAT_RGB565;
    display_config.buffer_height = 0;
    display_config.user_data = NULL;
    if (tinyui_display_set_default_config(&display_config) != 0) {
        tinyui_deinit();
        return 1;
    }

    tinyui_demo_v23_theme_layout_resource();

    if (tinyui_sdl_window_create(display_config.width, display_config.height) != 0
        || tinyui_sdl_mouse_create() != 0) {
        tinyui_sdl_quit();
        tinyui_deinit();
        return 1;
    }

    previous_ticks = SDL_GetTicks();
    for (;;) {
        unsigned int current_ticks = SDL_GetTicks();
        unsigned int elapsed_ms = current_ticks - previous_ticks;
        int step = tinyui_runtime_internal_timer_handler();

        previous_ticks = current_ticks;
        tinyui_demo_v23_theme_layout_resource_frame(elapsed_ms);

        if (step < 0) {
            tinyui_sdl_quit();
            tinyui_deinit();
            return 1;
        }
        if (step > 0) {
            tinyui_sdl_quit();
            tinyui_deinit();
            return 0;
        }
    }
}
