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
    if (picoui_backend_widget_bind_host(checkbox->widget.backend_widget, &checkbox->widget) != 0) {
        free(checkbox);
        return 0;
    }
    return checkbox;
}

struct picoui_checkbox *picoui_checkbox_create_with_props(struct picoui_window *parent,
                                                          const struct picoui_checkbox_props *props)
{
    struct picoui_checkbox *checkbox;
    struct picoui_backend_widget *backend;

    if (props == 0) {
        return 0;
    }

    checkbox = picoui_checkbox_create(parent, props->id);
    if (checkbox == 0) {
        return 0;
    }

    checkbox->checked = 0;
    checkbox->cb = 0;
    checkbox->user_data = 0;
    if (props->text != 0 && picoui_checkbox_set_text(checkbox, props->text) != 0) {
        free(checkbox);
        return 0;
    }
    backend = (struct picoui_backend_widget *)checkbox->widget.backend_widget;
    checkbox->checked = props->checked != 0;
    if (picoui_backend_widget_update_value(backend,
                                           checkbox->checked,
                                           0,
                                           &checkbox->widget,
                                           0) != 0) {
        free(checkbox);
        return 0;
    }
    backend->last_signal = PICOUI_BACKEND_SIGNAL_NONE;
    backend->dispatch_count = 0;
    checkbox->cb = props->on_toggled;
    checkbox->user_data = props->user_data;
    return checkbox;
}

int picoui_checkbox_set_checked(struct picoui_checkbox *checkbox, int checked)
{
    int normalized_checked;

    if (checkbox == 0) {
        return -1;
    }

    normalized_checked = checked != 0;
    if (checkbox->checked == normalized_checked) {
        return 0;
    }

    if (checkbox->widget.backend_widget == 0) {
        return -1;
    }

    checkbox->checked = normalized_checked;
    return picoui_backend_widget_update_value(checkbox->widget.backend_widget,
                                              checkbox->checked,
                                              checkbox->cb,
                                              &checkbox->widget,
                                              checkbox->user_data);
}

int picoui_checkbox_is_checked(struct picoui_checkbox *checkbox)
{
    if (checkbox == 0) {
        return 0;
    }

    return checkbox->checked;
}

int picoui_checkbox_set_text(struct picoui_checkbox *checkbox, const char *text)
{
    if (checkbox == 0 || text == 0) {
        return -1;
    }

    if (picoui_widget_set_text(&checkbox->widget, text) != 0) {
        return -1;
    }
    return picoui_backend_set_text(checkbox->widget.backend_widget, text);
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
