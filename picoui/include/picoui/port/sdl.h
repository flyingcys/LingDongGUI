#ifndef PICOUI_PORT_SDL_H
#define PICOUI_PORT_SDL_H

struct picoui_display_config;
struct picoui_indev;
struct picoui_sdl_runtime;
struct picoui_sdl_event {
    enum {
        PICOUI_PORT_EVENT_NONE = 0,
        PICOUI_PORT_EVENT_QUIT,
        PICOUI_PORT_EVENT_POINTER_DOWN,
        PICOUI_PORT_EVENT_POINTER_UP,
        PICOUI_PORT_EVENT_POINTER_MOTION,
        PICOUI_PORT_EVENT_EXPOSED,
    } type;
    int x;
    int y;
    unsigned int pressed;
};

int picoui_sdl_hal_init(int width, int height);
int picoui_port_sdl_default_pointer_indev(struct picoui_indev **out_indev);
struct picoui_app;
int picoui_port_sdl_attach(struct picoui_app *app);
unsigned int picoui_port_sdl_tick_get(void *user_data);
void picoui_port_sdl_delay(unsigned int ms, void *user_data);
int picoui_port_sdl_copy_default_display_config(struct picoui_display_config *out_config);
struct picoui_sdl_runtime *picoui_port_sdl_runtime_create(const char *title, int width, int height);
void picoui_port_sdl_runtime_destroy(struct picoui_sdl_runtime *runtime);
int picoui_port_sdl_runtime_get_window_size(const struct picoui_sdl_runtime *runtime,
                                            int *width,
                                            int *height);
int picoui_port_sdl_runtime_poll_event(struct picoui_sdl_runtime *runtime,
                                       struct picoui_sdl_event *event);
int picoui_port_sdl_runtime_present_argb8888(struct picoui_sdl_runtime *runtime,
                                             const unsigned int *pixels,
                                             int width,
                                             int height);

#endif
