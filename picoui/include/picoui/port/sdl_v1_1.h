#ifndef PICOUI_PORT_SDL_V1_1_H
#define PICOUI_PORT_SDL_V1_1_H

#include <stdint.h>

struct picoui_indev;
struct picoui_port_sdl_host;

enum picoui_port_sdl_event_type {
    PICOUI_PORT_SDL_EVENT_NONE = 0,
    PICOUI_PORT_SDL_EVENT_QUIT,
    PICOUI_PORT_SDL_EVENT_POINTER,
};

struct picoui_port_sdl_event {
    enum picoui_port_sdl_event_type type;
    int x;
    int y;
    int pressed;
};

int picoui_sdl_hal_init(int width, int height);
int picoui_port_sdl_default_pointer_indev(struct picoui_indev **out_indev);
struct picoui_port_sdl_host *picoui_port_sdl_host_create(void);
void picoui_port_sdl_host_destroy(struct picoui_port_sdl_host *host);
int picoui_port_sdl_host_ensure_window(struct picoui_port_sdl_host *host,
                                       const char *title,
                                       int width,
                                       int height);
int picoui_port_sdl_host_get_window_size(struct picoui_port_sdl_host *host, int *width, int *height);
int picoui_port_sdl_host_poll_event(struct picoui_port_sdl_host *host,
                                    struct picoui_port_sdl_event *event);
int picoui_port_sdl_host_present(struct picoui_port_sdl_host *host,
                                 const uint32_t *pixels,
                                 int width,
                                 int height,
                                 uint8_t clear_red,
                                 uint8_t clear_green,
                                 uint8_t clear_blue,
                                 uint8_t clear_alpha);

#endif
