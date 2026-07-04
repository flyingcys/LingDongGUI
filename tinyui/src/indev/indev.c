#include "internal.h"
#include "indev/indev.h"

#include <stddef.h>

static int tinyui_input_key_is_valid(enum tinyui_input_key key)
{
    return key >= TINYUI_INPUT_KEY_NONE && key <= TINYUI_INPUT_KEY_BACK;
}

int tinyui_input_set_read_callback(struct tinyui_app *app,
                                   tinyui_input_read_cb_t callback,
                                   void *user_data)
{
    if (app == NULL) {
        return -1;
    }

    app->input_port.read_callback = callback;
    app->input_port.read_user_data = user_data;
    return 0;
}

int tinyui_input_push_pointer(struct tinyui_app *app, int x, int y, int pressed)
{
    if (app == NULL) {
        return -1;
    }

    app->input_port.pointer_x = x;
    app->input_port.pointer_y = y;
    app->input_port.pointer_pressed = pressed ? 1 : 0;
    return 0;
}

int tinyui_input_get_pointer(const struct tinyui_app *app, int *x, int *y, int *pressed)
{
    if (app == NULL || x == NULL || y == NULL || pressed == NULL) {
        return -1;
    }

    *x = app->input_port.pointer_x;
    *y = app->input_port.pointer_y;
    *pressed = app->input_port.pointer_pressed;
    return 0;
}

int tinyui_input_push_key(struct tinyui_app *app, enum tinyui_input_key key, int pressed)
{
    if (app == NULL || !tinyui_input_key_is_valid(key)) {
        return -1;
    }

    app->input_port.key = key;
    app->input_port.key_pressed = pressed ? 1 : 0;
    return 0;
}

int tinyui_input_get_key(const struct tinyui_app *app,
                         enum tinyui_input_key *key,
                         int *pressed)
{
    if (app == NULL || key == NULL || pressed == NULL) {
        return -1;
    }

    *key = app->input_port.key;
    *pressed = app->input_port.key_pressed;
    return 0;
}
