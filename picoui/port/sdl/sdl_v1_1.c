#include "picoui/port/sdl_v1_1.h"

#include "picoui/display_v1_1.h"
#include "picoui/indev.h"

#include <SDL.h>
#include <stdlib.h>
#include <string.h>

static struct picoui_display *g_picoui_sdl_v1_1_display;
static struct picoui_indev *g_picoui_sdl_v1_1_pointer_indev;

struct picoui_port_sdl_host {
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Texture *texture;
    int width;
    int height;
};

static int picoui_port_sdl_init_runtime(void)
{
    if (SDL_WasInit(SDL_INIT_VIDEO | SDL_INIT_EVENTS) == 0
        && SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) != 0) {
        return -1;
    }

    return 0;
}

int picoui_sdl_hal_init(int width, int height)
{
    struct picoui_display *display;
    struct picoui_indev *pointer_indev;

    if (width <= 0 || height <= 0) {
        return -1;
    }

    if (picoui_port_sdl_init_runtime() != 0) {
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

    g_picoui_sdl_v1_1_display = display;
    g_picoui_sdl_v1_1_pointer_indev = pointer_indev;
    return 0;
}

int picoui_port_sdl_default_pointer_indev(struct picoui_indev **out_indev)
{
    if (out_indev == 0 || g_picoui_sdl_v1_1_pointer_indev == 0) {
        return -1;
    }

    *out_indev = g_picoui_sdl_v1_1_pointer_indev;
    return 0;
}

static void picoui_port_sdl_host_reset(struct picoui_port_sdl_host *host)
{
    if (host == NULL) {
        return;
    }

    if (host->texture != NULL) {
        SDL_DestroyTexture(host->texture);
    }
    if (host->renderer != NULL) {
        SDL_DestroyRenderer(host->renderer);
    }
    if (host->window != NULL) {
        SDL_DestroyWindow(host->window);
    }

    host->texture = NULL;
    host->renderer = NULL;
    host->window = NULL;
    host->width = 0;
    host->height = 0;
}

struct picoui_port_sdl_host *picoui_port_sdl_host_create(void)
{
    return (struct picoui_port_sdl_host *)calloc(1, sizeof(struct picoui_port_sdl_host));
}

void picoui_port_sdl_host_destroy(struct picoui_port_sdl_host *host)
{
    if (host == NULL) {
        return;
    }

    picoui_port_sdl_host_reset(host);
    free(host);
}

int picoui_port_sdl_host_ensure_window(struct picoui_port_sdl_host *host,
                                       const char *title,
                                       int width,
                                       int height)
{
    const char *window_title = title != NULL ? title : "PicoUI Demo";

    if (host == NULL || width <= 0 || height <= 0) {
        return -1;
    }
    if (picoui_port_sdl_init_runtime() != 0) {
        return -1;
    }
    if (host->window != NULL && host->renderer != NULL && host->texture != NULL
        && host->width == width && host->height == height) {
        return 0;
    }

    picoui_port_sdl_host_reset(host);
    host->window = SDL_CreateWindow(window_title,
                                    SDL_WINDOWPOS_CENTERED,
                                    SDL_WINDOWPOS_CENTERED,
                                    width,
                                    height,
                                    SDL_WINDOW_SHOWN);
    if (host->window == NULL) {
        return -1;
    }

    host->renderer = SDL_CreateRenderer(host->window, -1, SDL_RENDERER_ACCELERATED);
    if (host->renderer == NULL) {
        host->renderer = SDL_CreateRenderer(host->window, -1, SDL_RENDERER_SOFTWARE);
    }
    if (host->renderer == NULL) {
        picoui_port_sdl_host_reset(host);
        return -1;
    }

    host->texture = SDL_CreateTexture(host->renderer,
                                      SDL_PIXELFORMAT_ARGB8888,
                                      SDL_TEXTUREACCESS_STREAMING,
                                      width,
                                      height);
    if (host->texture == NULL) {
        picoui_port_sdl_host_reset(host);
        return -1;
    }

    host->width = width;
    host->height = height;
    return 0;
}

int picoui_port_sdl_host_get_window_size(struct picoui_port_sdl_host *host, int *width, int *height)
{
    if (host == NULL || host->window == NULL || width == NULL || height == NULL) {
        return -1;
    }

    SDL_GetWindowSize(host->window, width, height);
    return 0;
}

int picoui_port_sdl_host_poll_event(struct picoui_port_sdl_host *host,
                                    struct picoui_port_sdl_event *event)
{
    SDL_Event native_event;

    if (host == NULL || event == NULL) {
        return -1;
    }

    memset(event, 0, sizeof(*event));
    event->type = PICOUI_PORT_SDL_EVENT_NONE;
    while (SDL_PollEvent(&native_event)) {
        if (native_event.type == SDL_QUIT) {
            event->type = PICOUI_PORT_SDL_EVENT_QUIT;
            return 1;
        }
        if (native_event.type == SDL_MOUSEBUTTONDOWN
            && native_event.button.button == SDL_BUTTON_LEFT) {
            event->type = PICOUI_PORT_SDL_EVENT_POINTER;
            event->x = native_event.button.x;
            event->y = native_event.button.y;
            event->pressed = 1;
            return 1;
        }
        if (native_event.type == SDL_MOUSEBUTTONUP
            && native_event.button.button == SDL_BUTTON_LEFT) {
            event->type = PICOUI_PORT_SDL_EVENT_POINTER;
            event->x = native_event.button.x;
            event->y = native_event.button.y;
            event->pressed = 0;
            return 1;
        }
        if (native_event.type == SDL_MOUSEMOTION) {
            event->type = PICOUI_PORT_SDL_EVENT_POINTER;
            event->x = native_event.motion.x;
            event->y = native_event.motion.y;
            event->pressed = (native_event.motion.state & SDL_BUTTON_LMASK) != 0U;
            return 1;
        }
    }

    return 0;
}

int picoui_port_sdl_host_present(struct picoui_port_sdl_host *host,
                                 const uint32_t *pixels,
                                 int width,
                                 int height,
                                 uint8_t clear_red,
                                 uint8_t clear_green,
                                 uint8_t clear_blue,
                                 uint8_t clear_alpha)
{
    if (host == NULL || host->renderer == NULL || host->texture == NULL || pixels == NULL
        || width <= 0 || height <= 0) {
        return -1;
    }

    SDL_SetRenderDrawColor(host->renderer, clear_red, clear_green, clear_blue, clear_alpha);
    SDL_RenderClear(host->renderer);
    SDL_UpdateTexture(host->texture, NULL, pixels, (int)((size_t)width * sizeof(*pixels)));
    SDL_RenderCopy(host->renderer, host->texture, NULL, NULL);
    SDL_RenderPresent(host->renderer);
    return 0;
}
