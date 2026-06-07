#include "picoui/port/sdl.h"

#include "picoui/display.h"
#include "picoui/indev.h"
#include "picoui/osal.h"
#include "picoui/tick.h"

#include <SDL.h>
#include <stdlib.h>

static struct picoui_display *g_picoui_sdl_display;
static struct picoui_indev *g_picoui_sdl_pointer_indev;
struct picoui_sdl_runtime {
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Texture *texture;
    int width;
    int height;
};

unsigned int picoui_port_sdl_tick_get(void *user_data)
{
    (void)user_data;
    return (unsigned int)SDL_GetTicks();
}

void picoui_port_sdl_delay(unsigned int ms, void *user_data)
{
    (void)user_data;
    SDL_Delay((Uint32)ms);
}

int picoui_port_sdl_copy_default_display_config(struct picoui_display_config *out_config)
{
    int width = 480;
    int height = 320;

    if (out_config == 0) {
        return -1;
    }

    if (g_picoui_sdl_display != 0
        && picoui_display_get_size(g_picoui_sdl_display, &width, &height) != 0) {
        return -1;
    }

    out_config->width = width;
    out_config->height = height;
    out_config->color_format = PICOUI_COLOR_FORMAT_RGB565;
    out_config->buffer_height = 0;
    out_config->user_data = 0;
    return 0;
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
    struct picoui_display_config display = {0};

    if (app == NULL) {
        return -1;
    }

    if (picoui_port_sdl_copy_default_display_config(&display) != 0) {
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

struct picoui_sdl_runtime *picoui_port_sdl_runtime_create(const char *title, int width, int height)
{
    struct picoui_sdl_runtime *runtime;

    if (title == 0 || width <= 0 || height <= 0) {
        return 0;
    }
    if (SDL_WasInit(SDL_INIT_VIDEO | SDL_INIT_EVENTS) == 0
        && SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) != 0) {
        return 0;
    }

    runtime = calloc(1, sizeof(*runtime));
    if (runtime == 0) {
        return 0;
    }
    runtime->window = SDL_CreateWindow(title,
                                       SDL_WINDOWPOS_CENTERED,
                                       SDL_WINDOWPOS_CENTERED,
                                       width,
                                       height,
                                       SDL_WINDOW_SHOWN);
    if (runtime->window == 0) {
        free(runtime);
        return 0;
    }
    runtime->renderer = SDL_CreateRenderer(runtime->window, -1, SDL_RENDERER_ACCELERATED);
    if (runtime->renderer == 0) {
        runtime->renderer = SDL_CreateRenderer(runtime->window, -1, SDL_RENDERER_SOFTWARE);
    }
    if (runtime->renderer == 0) {
        SDL_DestroyWindow(runtime->window);
        free(runtime);
        return 0;
    }
    runtime->texture = SDL_CreateTexture(runtime->renderer,
                                         SDL_PIXELFORMAT_ARGB8888,
                                         SDL_TEXTUREACCESS_STREAMING,
                                         width,
                                         height);
    if (runtime->texture == 0) {
        SDL_DestroyRenderer(runtime->renderer);
        SDL_DestroyWindow(runtime->window);
        free(runtime);
        return 0;
    }
    runtime->width = width;
    runtime->height = height;
    return runtime;
}

void picoui_port_sdl_runtime_destroy(struct picoui_sdl_runtime *runtime)
{
    if (runtime == 0) {
        return;
    }
    if (runtime->texture != 0) {
        SDL_DestroyTexture(runtime->texture);
    }
    if (runtime->renderer != 0) {
        SDL_DestroyRenderer(runtime->renderer);
    }
    if (runtime->window != 0) {
        SDL_DestroyWindow(runtime->window);
    }
    free(runtime);
}

int picoui_port_sdl_runtime_get_window_size(const struct picoui_sdl_runtime *runtime,
                                            int *width,
                                            int *height)
{
    if (runtime == 0 || runtime->window == 0 || width == 0 || height == 0) {
        return -1;
    }
    SDL_GetWindowSize(runtime->window, width, height);
    return 0;
}

int picoui_port_sdl_runtime_poll_event(struct picoui_sdl_runtime *runtime,
                                       struct picoui_sdl_event *event)
{
    SDL_Event sdl_event;

    if (runtime == 0 || event == 0) {
        return -1;
    }
    if (SDL_PollEvent(&sdl_event) == 0) {
        event->type = PICOUI_PORT_EVENT_NONE;
        event->x = 0;
        event->y = 0;
        event->pressed = 0U;
        return 0;
    }

    event->x = 0;
    event->y = 0;
    event->pressed = 0U;
    switch (sdl_event.type) {
    case SDL_QUIT:
        event->type = PICOUI_PORT_EVENT_QUIT;
        return 1;
    case SDL_MOUSEBUTTONDOWN:
        if (sdl_event.button.button != SDL_BUTTON_LEFT) {
            event->type = PICOUI_PORT_EVENT_NONE;
            return 1;
        }
        event->type = PICOUI_PORT_EVENT_POINTER_DOWN;
        event->x = sdl_event.button.x;
        event->y = sdl_event.button.y;
        event->pressed = 1U;
        return 1;
    case SDL_MOUSEBUTTONUP:
        if (sdl_event.button.button != SDL_BUTTON_LEFT) {
            event->type = PICOUI_PORT_EVENT_NONE;
            return 1;
        }
        event->type = PICOUI_PORT_EVENT_POINTER_UP;
        event->x = sdl_event.button.x;
        event->y = sdl_event.button.y;
        return 1;
    case SDL_MOUSEMOTION:
        event->type = PICOUI_PORT_EVENT_POINTER_MOTION;
        event->x = sdl_event.motion.x;
        event->y = sdl_event.motion.y;
        event->pressed = (sdl_event.motion.state & SDL_BUTTON_LMASK) != 0U ? 1U : 0U;
        return 1;
    case SDL_WINDOWEVENT:
        event->type = sdl_event.window.event == SDL_WINDOWEVENT_EXPOSED
                          ? PICOUI_PORT_EVENT_EXPOSED
                          : PICOUI_PORT_EVENT_NONE;
        return 1;
    default:
        event->type = PICOUI_PORT_EVENT_NONE;
        return 1;
    }
}

int picoui_port_sdl_runtime_present_argb8888(struct picoui_sdl_runtime *runtime,
                                             const unsigned int *pixels,
                                             int width,
                                             int height)
{
    if (runtime == 0 || runtime->renderer == 0 || runtime->texture == 0 || pixels == 0) {
        return -1;
    }
    if (width != runtime->width || height != runtime->height) {
        return -1;
    }
    if (SDL_UpdateTexture(runtime->texture, 0, pixels, (int)(width * (int)sizeof(*pixels))) != 0) {
        return -1;
    }
    if (SDL_RenderClear(runtime->renderer) != 0) {
        return -1;
    }
    if (SDL_RenderCopy(runtime->renderer, runtime->texture, 0, 0) != 0) {
        return -1;
    }
    SDL_RenderPresent(runtime->renderer);
    return 0;
}
