#ifndef PICOUI_PORT_SDL_H
#define PICOUI_PORT_SDL_H

struct picoui_indev;

int picoui_sdl_hal_init(int width, int height);
int picoui_port_sdl_default_pointer_indev(struct picoui_indev **out_indev);
struct picoui_app;
int picoui_port_sdl_attach(struct picoui_app *app);

#endif
