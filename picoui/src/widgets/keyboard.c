#include "picoui/keyboard.h"
#include "internal.h"

#include <stdlib.h>

static int picoui_keyboard_props_are_valid(const struct picoui_keyboard_props *props)
{
    return props != 0
        && props->id != 0
        && props->width >= 0
        && props->height >= 0
        && props->radius >= 0
        && props->padding >= 0;
}

struct picoui_keyboard *picoui_keyboard_create(struct picoui_window *parent, const char *id)
{
    struct picoui_keyboard *keyboard;

    if (parent == 0 || id == 0) {
        return 0;
    }

    keyboard = calloc(1, sizeof(*keyboard));
    if (keyboard == 0) {
        return 0;
    }

    keyboard->widget.backend_widget = picoui_backend_create_keyboard(parent->widget.backend_widget, id);
    if (keyboard->widget.backend_widget == 0) {
        free(keyboard);
        return 0;
    }

    keyboard->id = id;
    keyboard->widget.visible = 1;
    keyboard->widget.enabled = 1;
    if (picoui_backend_widget_bind_host(keyboard->widget.backend_widget, &keyboard->widget) != 0) {
        free(keyboard);
        return 0;
    }
    return keyboard;
}

struct picoui_keyboard *picoui_keyboard_create_with_props(struct picoui_window *parent,
                                                          const struct picoui_keyboard_props *props)
{
    struct picoui_keyboard *keyboard;

    if (!picoui_keyboard_props_are_valid(props)) {
        return 0;
    }

    keyboard = picoui_keyboard_create(parent, props->id);
    if (keyboard == 0) {
        return 0;
    }

    if (picoui_widget_set_user_data(&keyboard->widget, props->user_data) != 0
        || picoui_widget_set_bg_color(&keyboard->widget, props->bg_color) != 0
        || picoui_widget_set_text_color(&keyboard->widget, props->text_color) != 0
        || picoui_widget_set_border_color(&keyboard->widget, props->border_color) != 0
        || picoui_widget_set_radius(&keyboard->widget, props->radius) != 0
        || picoui_widget_set_padding(&keyboard->widget, props->padding) != 0) {
        free(keyboard);
        return 0;
    }
    if (props->style_class != 0
        && picoui_widget_set_style_class(&keyboard->widget, props->style_class) != 0) {
        free(keyboard);
        return 0;
    }
    if ((props->width > 0 || props->height > 0)
        && picoui_widget_set_size(&keyboard->widget, props->width, props->height) != 0) {
        free(keyboard);
        return 0;
    }

    return keyboard;
}

int picoui_keyboard_input_ascii(struct picoui_keyboard *keyboard, unsigned int ascii)
{
    if (keyboard == 0) {
        return -1;
    }

    return picoui_backend_keyboard_input_ascii(keyboard->widget.backend_widget, ascii);
}

int picoui_keyboard_navigate(struct picoui_keyboard *keyboard, int direction)
{
    if (keyboard == 0) {
        return -1;
    }

    return picoui_backend_keyboard_navigate(keyboard->widget.backend_widget, direction);
}

int picoui_keyboard_update(struct picoui_keyboard *keyboard)
{
    if (keyboard == 0) {
        return -1;
    }

    return picoui_backend_keyboard_update(keyboard->widget.backend_widget);
}

int picoui_keyboard_button_update(struct picoui_keyboard *keyboard, unsigned int key_code)
{
    if (keyboard == 0 || key_code > 0xFFU) {
        return -1;
    }

    return picoui_backend_keyboard_button_update(keyboard->widget.backend_widget, (unsigned char)key_code);
}

int picoui_keyboard_click(struct picoui_keyboard *keyboard)
{
    if (keyboard == 0) {
        return -1;
    }

    return picoui_backend_keyboard_click(keyboard->widget.backend_widget);
}

int picoui_keyboard_exit(struct picoui_keyboard *keyboard)
{
    if (keyboard == 0) {
        return -1;
    }

    return picoui_backend_keyboard_exit(keyboard->widget.backend_widget);
}
