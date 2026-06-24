#ifndef TINYUI_KEYBOARD_H
#define TINYUI_KEYBOARD_H

#include "core/widget.h"

struct tinyui_window;
struct tinyui_keyboard;

struct tinyui_keyboard_button {
    int x;
    int y;
    int width;
    int height;
    const char *text;
    unsigned int key_code;
    unsigned int press_color;
    unsigned int release_color;
};

typedef void (*tinyui_keyboard_event_cb)(struct tinyui_keyboard *keyboard,
                                         unsigned int key_code,
                                         enum tinyui_native_signal signal,
                                         void *user_data);
typedef void (*tinyui_keyboard_draw_cb)(struct tinyui_keyboard *keyboard,
                                        const struct tinyui_keyboard_button *button,
                                        void *user_data);

struct tinyui_keyboard_props {
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

struct tinyui_keyboard *tinyui_keyboard_create(struct tinyui_window *parent, const char *id);

struct tinyui_keyboard *tinyui_keyboard_create_with_props(
    struct tinyui_window *parent,
    const struct tinyui_keyboard_props *props);

int tinyui_keyboard_input_ascii(struct tinyui_keyboard *keyboard, unsigned int ascii);

int tinyui_keyboard_navigate(struct tinyui_keyboard *keyboard, int direction);

int tinyui_keyboard_update(struct tinyui_keyboard *keyboard);

int tinyui_keyboard_button_update(struct tinyui_keyboard *keyboard, unsigned int key_code);

int tinyui_keyboard_click(struct tinyui_keyboard *keyboard);

int tinyui_keyboard_exit(struct tinyui_keyboard *keyboard);

int tinyui_keyboard_set_buttons(struct tinyui_keyboard *keyboard,
                                const struct tinyui_keyboard_button *buttons,
                                int count);

int tinyui_keyboard_get_buttons(const struct tinyui_keyboard *keyboard,
                                const struct tinyui_keyboard_button **buttons,
                                int *count);

int tinyui_keyboard_set_on_key_event(struct tinyui_keyboard *keyboard,
                                     tinyui_keyboard_event_cb cb,
                                     void *user_data);

int tinyui_keyboard_get_selected_key_code(const struct tinyui_keyboard *keyboard);

int tinyui_keyboard_set_layout(struct tinyui_keyboard *keyboard,
                               const struct tinyui_keyboard_button *buttons,
                               int count);

int tinyui_keyboard_set_event_callback(struct tinyui_keyboard *keyboard,
                                       tinyui_keyboard_event_cb cb,
                                       void *user_data);

int tinyui_keyboard_set_draw_callback(struct tinyui_keyboard *keyboard,
                                      tinyui_keyboard_draw_cb cb,
                                      void *user_data);

#endif
