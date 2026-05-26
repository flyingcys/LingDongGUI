#include "internal.h"
#include "picoui/checkbox.h"

#include <stdlib.h>

struct picoui_checkbox *picoui_checkbox_create(struct picoui_window *parent, const char *id)
{
    struct picoui_checkbox *checkbox;

    if (parent == 0 || id == 0) {
        return 0;
    }

    checkbox = calloc(1, sizeof(*checkbox));
    if (checkbox == 0) {
        return 0;
    }

    checkbox->widget.backend_widget = picoui_backend_create_checkbox(parent->widget.backend_widget, id);
    if (checkbox->widget.backend_widget == 0) {
        free(checkbox);
        return 0;
    }

    checkbox->id = id;
    checkbox->widget.visible = 1;
    checkbox->widget.enabled = 1;
    return checkbox;
}

struct picoui_checkbox *picoui_checkbox_create_with_props(struct picoui_window *parent,
                                                          const struct picoui_checkbox_props *props)
{
    struct picoui_checkbox *checkbox;

    if (props == 0) {
        return 0;
    }

    checkbox = picoui_checkbox_create(parent, props->id);
    if (checkbox == 0) {
        return 0;
    }

    checkbox->cb = props->on_toggled;
    checkbox->user_data = props->user_data;
    if (picoui_checkbox_set_checked(checkbox, props->checked) != 0) {
        free(checkbox);
        return 0;
    }

    return checkbox;
}

int picoui_checkbox_set_checked(struct picoui_checkbox *checkbox, int checked)
{
    if (checkbox == 0) {
        return -1;
    }

    checkbox->checked = checked != 0;
    picoui_backend_emit_value_changed(checkbox->cb,
                                      &checkbox->widget,
                                      checkbox->checked,
                                      checkbox->user_data);
    return 0;
}

int picoui_checkbox_is_checked(struct picoui_checkbox *checkbox)
{
    if (checkbox == 0) {
        return 0;
    }

    return checkbox->checked;
}

int picoui_checkbox_set_on_toggled(struct picoui_checkbox *checkbox,
                                   picoui_value_changed_cb cb,
                                   void *user_data)
{
    if (checkbox == 0) {
        return -1;
    }

    checkbox->cb = cb;
    checkbox->user_data = user_data;
    return 0;
}
