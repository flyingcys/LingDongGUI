#ifndef TINYUI_KEYBOARD_H
#define TINYUI_KEYBOARD_H

#include "widget.h"

struct picoui_window;
struct picoui_keyboard;

struct picoui_keyboard_button {
    int x;
    int y;
    int width;
    int height;
    const char *text;
    unsigned int key_code;
    unsigned int press_color;
    unsigned int release_color;
};

typedef void (*picoui_keyboard_event_cb)(struct picoui_keyboard *keyboard,
                                         unsigned int key_code,
                                         enum picoui_native_signal signal,
                                         void *user_data);
typedef void (*picoui_keyboard_draw_cb)(struct picoui_keyboard *keyboard,
                                        const struct picoui_keyboard_button *button,
                                        void *user_data);

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

struct picoui_keyboard *picoui_keyboard_create_with_props(
    struct picoui_window *parent,
    const struct picoui_keyboard_props *props);

int picoui_keyboard_input_ascii(struct picoui_keyboard *keyboard, unsigned int ascii);

int picoui_keyboard_navigate(struct picoui_keyboard *keyboard, int direction);

int picoui_keyboard_update(struct picoui_keyboard *keyboard);

int picoui_keyboard_button_update(struct picoui_keyboard *keyboard, unsigned int key_code);

int picoui_keyboard_click(struct picoui_keyboard *keyboard);

int picoui_keyboard_exit(struct picoui_keyboard *keyboard);

int picoui_keyboard_set_buttons(struct picoui_keyboard *keyboard,
                                const struct picoui_keyboard_button *buttons,
                                int count);

int picoui_keyboard_get_buttons(const struct picoui_keyboard *keyboard,
                                const struct picoui_keyboard_button **buttons,
                                int *count);

int picoui_keyboard_set_on_key_event(struct picoui_keyboard *keyboard,
                                     picoui_keyboard_event_cb cb,
                                     void *user_data);

int picoui_keyboard_get_selected_key_code(const struct picoui_keyboard *keyboard);

int picoui_keyboard_set_layout(struct picoui_keyboard *keyboard,
                               const struct picoui_keyboard_button *buttons,
                               int count);

int picoui_keyboard_set_event_callback(struct picoui_keyboard *keyboard,
                                       picoui_keyboard_event_cb cb,
                                       void *user_data);

int picoui_keyboard_set_draw_callback(struct picoui_keyboard *keyboard,
                                      picoui_keyboard_draw_cb cb,
                                      void *user_data);

#endif
