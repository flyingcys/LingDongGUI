#ifndef PICOUI_PORT_INPUT_H
#define PICOUI_PORT_INPUT_H

struct picoui_app;
struct picoui_indev;

enum picoui_input_key {
    PICOUI_INPUT_KEY_NONE = 0,
    PICOUI_INPUT_KEY_LEFT,
    PICOUI_INPUT_KEY_RIGHT,
    PICOUI_INPUT_KEY_UP,
    PICOUI_INPUT_KEY_DOWN,
    PICOUI_INPUT_KEY_ENTER,
    PICOUI_INPUT_KEY_BACK,
};

enum picoui_indev_type {
    PICOUI_INDEV_TYPE_NONE = 0,
    PICOUI_INDEV_TYPE_POINTER,
    PICOUI_INDEV_TYPE_KEYPAD,
    PICOUI_INDEV_TYPE_ENCODER,
};

struct picoui_indev_data {
    int pointer_x;
    int pointer_y;
    int pressed;
    enum picoui_input_key key;
};

typedef void (*picoui_indev_read_cb_t)(struct picoui_indev *indev,
                                       struct picoui_indev_data *data,
                                       void *user_data);

struct picoui_indev *picoui_indev_create(void);
int picoui_indev_set_type(struct picoui_indev *indev, enum picoui_indev_type type);
int picoui_indev_set_read_cb(struct picoui_indev *indev,
                             picoui_indev_read_cb_t callback,
                             void *user_data);

int picoui_input_push_pointer(struct picoui_app *app, int x, int y, int pressed);
int picoui_input_get_pointer(const struct picoui_app *app, int *x, int *y, int *pressed);
int picoui_input_push_key(struct picoui_app *app, enum picoui_input_key key, int pressed);
int picoui_input_get_key(const struct picoui_app *app,
                         enum picoui_input_key *key,
                         int *pressed);

#endif
