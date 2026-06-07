#include "runtime_state.h"
#include "internal_v1_1.h"

#include "picoui/button.h"

#include <stdlib.h>

struct picoui_button *picoui_button_create(struct picoui_window *parent, const char *id)
{
    struct picoui_button *button;

    if (parent == 0 || id == 0) {
        return 0;
    }

    button = calloc(1, sizeof(*button));
    if (button == 0) {
        return 0;
    }

    button->id = id;
    button->widget.visible = 1;
    button->widget.enabled = 1;

    if (picoui_v1_1_widget_append_child(&parent->widget, &button->widget) != 0) {
        free(button);
        return 0;
    }

    return button;
}

int picoui_button_set_on_clicked(struct picoui_button *button,
                                 picoui_event_cb cb,
                                 void *user_data)
{
    if (button == 0) {
        return -1;
    }

    button->on_clicked = cb;
    button->clicked_user_data = user_data;
    return 0;
}

int picoui_button_set_pressed(struct picoui_button *button, int pressed)
{
    int next_pressed;
    int was_pressed;

    if (button == 0) {
        return -1;
    }

    next_pressed = pressed ? 1 : 0;
    was_pressed = button->pressed;
    button->pressed = next_pressed;
    button->widget.dirty = 1;

    if (was_pressed == 1 && next_pressed == 0 && button->on_clicked != 0) {
        button->on_clicked((struct picoui_widget *)button, button->clicked_user_data);
    }

    return 0;
}

int picoui_button_get_pressed(struct picoui_button *button, int *pressed)
{
    if (button == 0 || pressed == 0) {
        return -1;
    }

    *pressed = button->pressed;
    return 0;
}
