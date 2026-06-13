#ifndef TINYUI_INDEV_H
#define TINYUI_INDEV_H

struct tinyui_app;

enum tinyui_input_key {
    TINYUI_INPUT_KEY_NONE = 0,
    TINYUI_INPUT_KEY_LEFT,
    TINYUI_INPUT_KEY_RIGHT,
    TINYUI_INPUT_KEY_UP,
    TINYUI_INPUT_KEY_DOWN,
    TINYUI_INPUT_KEY_ENTER,
    TINYUI_INPUT_KEY_BACK,
};

int tinyui_input_push_pointer(struct tinyui_app *app, int x, int y, int pressed);
int tinyui_input_get_pointer(const struct tinyui_app *app, int *x, int *y, int *pressed);
int tinyui_input_push_key(struct tinyui_app *app, enum tinyui_input_key key, int pressed);
int tinyui_input_get_key(const struct tinyui_app *app,
                         enum tinyui_input_key *key,
                         int *pressed);

#endif
