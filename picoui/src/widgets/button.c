#include "internal.h"
#include "picoui/button.h"

#include <stdlib.h>

static struct picoui_button *picoui_button_alloc(struct picoui_window *parent, const char *id)
{
    struct picoui_button *button;

    if (parent == 0 || id == 0) {
        return 0;
    }

    button = calloc(1, sizeof(*button));
    if (button == 0) {
        return 0;
    }

    button->widget.backend_widget = picoui_backend_create_button(parent->widget.backend_widget, id);
    if (button->widget.backend_widget == 0) {
        free(button);
        return 0;
    }

    button->id = id;
    button->widget.visible = 1;
    button->widget.enabled = 1;
    if (picoui_backend_widget_bind_host(button->widget.backend_widget, &button->widget) != 0) {
        free(button);
        return 0;
    }
    return button;
}

struct picoui_button *picoui_button_create(struct picoui_window *parent, const char *id)
{
    return picoui_button_alloc(parent, id);
}

struct picoui_button *picoui_button_create_with_props(struct picoui_window *parent,
                                                      const struct picoui_button_props *props)
{
    struct picoui_button *button;

    if (props == 0) {
        return 0;
    }

    button = picoui_button_alloc(parent, props->id);
    if (button == 0) {
        return 0;
    }

    button->on_clicked = props->on_clicked;
    button->user_data = props->user_data;
    if (props->text != 0 && picoui_button_set_text(button, props->text) != 0) {
        free(button);
        return 0;
    }
    if ((props->width > 0 || props->height > 0)
        && picoui_widget_set_size(&button->widget, props->width, props->height) != 0) {
        free(button);
        return 0;
    }

    return button;
}

static int picoui_button_set_event(struct picoui_button *button,
                                   picoui_event_cb cb,
                                   void *user_data,
                                   int kind)
{
    if (button == 0) {
        return -1;
    }

    if (kind == 0) {
        button->on_pressed = cb;
        button->on_pressed_user_data = user_data;
    } else if (kind == 1) {
        button->on_released = cb;
        button->on_released_user_data = user_data;
    } else {
        return -1;
    }
    return 0;
}

int picoui_button_set_text(struct picoui_button *button, const char *text)
{
    if (button == 0 || text == 0) {
        return -1;
    }

    if (picoui_widget_set_text(&button->widget, text) != 0) {
        return -1;
    }
    return picoui_backend_set_text(button->widget.backend_widget, text);
}

int picoui_button_set_on_clicked(struct picoui_button *button,
                                 picoui_event_cb cb,
                                 void *user_data)
{
    if (button == 0) {
        return -1;
    }

    button->on_clicked = cb;
    button->user_data = user_data;
    return 0;
}

int picoui_button_set_on_pressed(struct picoui_button *button,
                                 picoui_event_cb cb,
                                 void *user_data)
{
    return picoui_button_set_event(button, cb, user_data, 0);
}

int picoui_button_set_on_released(struct picoui_button *button,
                                  picoui_event_cb cb,
                                  void *user_data)
{
    return picoui_button_set_event(button, cb, user_data, 1);
}
