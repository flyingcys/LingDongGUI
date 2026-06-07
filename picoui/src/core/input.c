#include "runtime_state.h"
#include "picoui/input.h"

static int picoui_input_key_is_valid(enum picoui_input_key key)
{
    return key >= PICOUI_INPUT_KEY_NONE && key <= PICOUI_INPUT_KEY_BACK;
}

struct picoui_indev *picoui_indev_create(void)
{
    return &picoui_runtime_state()->default_indev;
}

int picoui_indev_set_type(struct picoui_indev *indev, enum picoui_indev_type type)
{
    if (indev == 0) {
        return -1;
    }

    indev->type = type;
    return 0;
}

int picoui_indev_set_read_cb(struct picoui_indev *indev,
                             picoui_indev_read_cb_t callback,
                             void *user_data)
{
    if (indev == 0 || callback == 0) {
        return -1;
    }

    indev->read_cb = callback;
    indev->read_user_data = user_data;
    return 0;
}

int picoui_input_push_pointer(struct picoui_app *app, int x, int y, int pressed)
{
    struct picoui_runtime_state *state = picoui_runtime_state();

    (void)app;
    if (!state->initialized) {
        return -1;
    }

    state->pointer_x = x;
    state->pointer_y = y;
    state->pointer_pressed = pressed ? 1 : 0;
    return 0;
}

int picoui_input_get_pointer(const struct picoui_app *app, int *x, int *y, int *pressed)
{
    const struct picoui_runtime_state *state = picoui_runtime_state();

    (void)app;
    if (!state->initialized || x == 0 || y == 0 || pressed == 0) {
        return -1;
    }

    *x = state->pointer_x;
    *y = state->pointer_y;
    *pressed = state->pointer_pressed;
    return 0;
}

int picoui_input_push_key(struct picoui_app *app, enum picoui_input_key key, int pressed)
{
    struct picoui_runtime_state *state = picoui_runtime_state();

    (void)app;
    if (!state->initialized || !picoui_input_key_is_valid(key)) {
        return -1;
    }

    state->key = key;
    state->key_pressed = pressed ? 1 : 0;
    return 0;
}

int picoui_input_get_key(const struct picoui_app *app,
                         enum picoui_input_key *key,
                         int *pressed)
{
    const struct picoui_runtime_state *state = picoui_runtime_state();

    (void)app;
    if (!state->initialized || key == 0 || pressed == 0) {
        return -1;
    }

    *key = state->key;
    *pressed = state->key_pressed;
    return 0;
}
