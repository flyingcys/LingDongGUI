#include "internal.h"
#include "indev.h"

#include <stddef.h>

static int tinyui_input_key_is_valid(enum picoui_input_key key)
{
    return key >= PICOUI_INPUT_KEY_NONE && key <= PICOUI_INPUT_KEY_BACK;
}

int picoui_input_push_pointer(struct picoui_app *app, int x, int y, int pressed)
{
    if (app == NULL) {
        return -1;
    }

    app->input_port.pointer_x = x;
    app->input_port.pointer_y = y;
    app->input_port.pointer_pressed = pressed ? 1 : 0;
    return 0;
}

int picoui_input_get_pointer(const struct picoui_app *app, int *x, int *y, int *pressed)
{
    if (app == NULL || x == NULL || y == NULL || pressed == NULL) {
        return -1;
    }

    *x = app->input_port.pointer_x;
    *y = app->input_port.pointer_y;
    *pressed = app->input_port.pointer_pressed;
    return 0;
}

int picoui_input_push_key(struct picoui_app *app, enum picoui_input_key key, int pressed)
{
    if (app == NULL || !tinyui_input_key_is_valid(key)) {
        return -1;
    }

    app->input_port.key = key;
    app->input_port.key_pressed = pressed ? 1 : 0;
    return 0;
}

int picoui_input_get_key(const struct picoui_app *app,
                         enum picoui_input_key *key,
                         int *pressed)
{
    if (app == NULL || key == NULL || pressed == NULL) {
        return -1;
    }

    *key = app->input_port.key;
    *pressed = app->input_port.key_pressed;
    return 0;
}
