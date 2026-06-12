#ifndef PICOUI_PORT_SDL_H
#define PICOUI_PORT_SDL_H

/* Transitional compatibility shim retained until SDL-facing callers migrate off picoui/. */

struct picoui_app;

int picoui_port_sdl_attach(struct picoui_app *app);

#endif
