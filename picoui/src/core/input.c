#include "runtime_state.h"
#include "picoui/indev.h"

#include <stdlib.h>

struct picoui_indev {
    enum picoui_indev_type type;
    picoui_indev_read_cb_t read_cb;
    void *read_user_data;
};

struct picoui_indev *picoui_indev_create(void)
{
    struct picoui_runtime_state *state = picoui_runtime_state();
    struct picoui_indev *indev = (struct picoui_indev *)calloc(1, sizeof(struct picoui_indev));

    if (indev == 0) {
        return 0;
    }
    if (state->default_indev == 0) {
        state->default_indev = indev;
    }
    return indev;
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
