#include "internal.h"
#include "picoui/line_edit.h"

#include <stdlib.h>

static int picoui_line_edit_type_is_valid(enum picoui_line_edit_type type)
{
    return type >= PICOUI_LINE_EDIT_TYPE_STRING && type <= PICOUI_LINE_EDIT_TYPE_FLOAT;
}

static int picoui_line_edit_keyboard_binding_is_valid(unsigned int keyboard_binding)
{
    return keyboard_binding > 0U && keyboard_binding <= 0xFFFFU;
}

static int picoui_line_edit_props_are_valid(const struct picoui_line_edit_props *props)
{
    return props != 0
        && props->id != 0
        && props->width >= 0
        && props->height >= 0
        && props->radius >= 0
        && props->padding >= 0
        && (props->has_type == 0 || picoui_line_edit_type_is_valid(props->type))
        && (props->has_keyboard_binding == 0
            || picoui_line_edit_keyboard_binding_is_valid(props->keyboard_binding));
}

struct picoui_line_edit *picoui_line_edit_create(struct picoui_window *parent, const char *id)
{
    struct picoui_line_edit *line_edit;

    if (parent == 0 || id == 0) {
        return 0;
    }

    line_edit = calloc(1, sizeof(*line_edit));
    if (line_edit == 0) {
        return 0;
    }

    line_edit->widget.backend_widget =
        picoui_backend_create_line_edit(parent->widget.backend_widget, id);
    if (line_edit->widget.backend_widget == 0) {
        free(line_edit);
        return 0;
    }

    line_edit->id = id;
    line_edit->type = PICOUI_LINE_EDIT_TYPE_STRING;
    line_edit->widget.visible = 1;
    line_edit->widget.enabled = 1;
    if (picoui_backend_widget_bind_host(line_edit->widget.backend_widget, &line_edit->widget) != 0) {
        free(line_edit);
        return 0;
    }
    if (picoui_backend_line_edit_bind_host(line_edit->widget.backend_widget) != 0) {
        free(line_edit);
        return 0;
    }
    return line_edit;
}

struct picoui_line_edit *picoui_line_edit_create_with_props(struct picoui_window *parent,
                                                            const struct picoui_line_edit_props *props)
{
    struct picoui_line_edit *line_edit;

    if (!picoui_line_edit_props_are_valid(props)) {
        return 0;
    }

    line_edit = picoui_line_edit_create(parent, props->id);
    if (line_edit == 0) {
        return 0;
    }

    if ((props->text != 0 && picoui_line_edit_set_text(line_edit, props->text) != 0)
        || (props->has_type != 0 && picoui_line_edit_set_type(line_edit, props->type) != 0)
        || (props->has_keyboard_binding != 0
            && picoui_line_edit_set_keyboard_binding(line_edit, props->keyboard_binding) != 0)) {
        free(line_edit);
        return 0;
    }

    if (picoui_widget_set_user_data(&line_edit->widget, props->user_data) != 0
        || picoui_widget_set_bg_color(&line_edit->widget, props->bg_color) != 0
        || picoui_widget_set_text_color(&line_edit->widget, props->text_color) != 0
        || picoui_widget_set_border_color(&line_edit->widget, props->border_color) != 0
        || picoui_widget_set_radius(&line_edit->widget, props->radius) != 0
        || picoui_widget_set_padding(&line_edit->widget, props->padding) != 0) {
        free(line_edit);
        return 0;
    }
    if (props->style_class != 0
        && picoui_widget_set_style_class(&line_edit->widget, props->style_class) != 0) {
        free(line_edit);
        return 0;
    }
    if ((props->width > 0 || props->height > 0)
        && picoui_widget_set_size(&line_edit->widget, props->width, props->height) != 0) {
        free(line_edit);
        return 0;
    }

    return line_edit;
}

int picoui_line_edit_set_text(struct picoui_line_edit *line_edit, const char *text)
{
    if (line_edit == 0 || text == 0) {
        return -1;
    }

    if (picoui_widget_set_text(&line_edit->widget, text) != 0) {
        return -1;
    }

    return picoui_backend_line_edit_set_text(line_edit->widget.backend_widget, text);
}

const char *picoui_line_edit_get_text(const struct picoui_line_edit *line_edit)
{
    const char *backend_text;

    if (line_edit == 0) {
        return 0;
    }

    backend_text = picoui_backend_line_edit_get_text((void *)line_edit->widget.backend_widget);
    if (backend_text != 0) {
        return backend_text;
    }

    return line_edit->widget.text;
}

int picoui_line_edit_set_type(struct picoui_line_edit *line_edit, enum picoui_line_edit_type type)
{
    if (line_edit == 0 || !picoui_line_edit_type_is_valid(type)) {
        return -1;
    }

    if (picoui_backend_line_edit_set_type(line_edit->widget.backend_widget, type) != 0) {
        return -1;
    }

    line_edit->type = type;
    return 0;
}

int picoui_line_edit_get_type(const struct picoui_line_edit *line_edit,
                              enum picoui_line_edit_type *type)
{
    if (line_edit == 0 || type == 0) {
        return -1;
    }

    return picoui_backend_line_edit_get_type((void *)line_edit->widget.backend_widget, type);
}

int picoui_line_edit_set_keyboard_binding(struct picoui_line_edit *line_edit,
                                          unsigned int keyboard_binding)
{
    if (line_edit == 0 || !picoui_line_edit_keyboard_binding_is_valid(keyboard_binding)) {
        return -1;
    }

    if (picoui_backend_line_edit_set_keyboard_binding(line_edit->widget.backend_widget,
                                                      keyboard_binding) != 0) {
        return -1;
    }

    line_edit->keyboard_binding = keyboard_binding;
    return 0;
}

int picoui_line_edit_get_keyboard_binding(const struct picoui_line_edit *line_edit,
                                          unsigned int *keyboard_binding)
{
    if (line_edit == 0 || keyboard_binding == 0) {
        return -1;
    }

    return picoui_backend_line_edit_get_keyboard_binding((void *)line_edit->widget.backend_widget,
                                                         keyboard_binding);
}

int picoui_line_edit_get_editing(const struct picoui_line_edit *line_edit, int *editing)
{
    if (line_edit == 0 || editing == 0) {
        return -1;
    }

    return picoui_backend_line_edit_get_editing((void *)line_edit->widget.backend_widget, editing);
}

int picoui_line_edit_set_on_edit_finished(struct picoui_line_edit *line_edit,
                                          picoui_line_edit_finished_cb cb,
                                          void *user_data)
{
    if (line_edit == 0) {
        return -1;
    }

    line_edit->on_edit_finished = cb;
    line_edit->on_edit_finished_user_data = user_data;
    return 0;
}
