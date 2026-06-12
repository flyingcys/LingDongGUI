#ifndef TINYUI_INDEV_H
#define TINYUI_INDEV_H

struct picoui_app;

enum picoui_input_key {
    PICOUI_INPUT_KEY_NONE = 0,
    PICOUI_INPUT_KEY_LEFT,
    PICOUI_INPUT_KEY_RIGHT,
    PICOUI_INPUT_KEY_UP,
    PICOUI_INPUT_KEY_DOWN,
    PICOUI_INPUT_KEY_ENTER,
    PICOUI_INPUT_KEY_BACK,
};

int picoui_input_push_pointer(struct picoui_app *app, int x, int y, int pressed);
int picoui_input_get_pointer(const struct picoui_app *app, int *x, int *y, int *pressed);
int picoui_input_push_key(struct picoui_app *app, enum picoui_input_key key, int pressed);
int picoui_input_get_key(const struct picoui_app *app,
                         enum picoui_input_key *key,
                         int *pressed);

#endif
