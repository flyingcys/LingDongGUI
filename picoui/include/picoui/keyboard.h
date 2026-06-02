#ifndef PICOUI_KEYBOARD_H
#define PICOUI_KEYBOARD_H

#include "picoui/widget.h"

struct picoui_window;
struct picoui_keyboard;

struct picoui_keyboard_props {
    const char *id;
    const char *style_class;
    void *user_data;
    int width;
    int height;
    unsigned int bg_color;
    unsigned int text_color;
    unsigned int border_color;
    int radius;
    int padding;
};

struct picoui_keyboard *picoui_keyboard_create(struct picoui_window *parent, const char *id);
struct picoui_keyboard *picoui_keyboard_create_with_props(struct picoui_window *parent,
                                                          const struct picoui_keyboard_props *props);
int picoui_keyboard_input_ascii(struct picoui_keyboard *keyboard, unsigned int ascii);
int picoui_keyboard_navigate(struct picoui_keyboard *keyboard, int direction);
int picoui_keyboard_update(struct picoui_keyboard *keyboard);
int picoui_keyboard_button_update(struct picoui_keyboard *keyboard, unsigned int key_code);
int picoui_keyboard_click(struct picoui_keyboard *keyboard);
int picoui_keyboard_exit(struct picoui_keyboard *keyboard);

#endif
