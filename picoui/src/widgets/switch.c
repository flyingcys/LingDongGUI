#include "internal.h"
#include "picoui/switch.h"

#include <stdlib.h>

static int picoui_switch_props_are_valid(const struct picoui_switch_props *props)
{
    return props != 0
        && props->id != 0
        && props->width >= 0
        && props->height >= 0
        && props->radius >= 0
        && props->padding >= 0;
}

struct picoui_switch *picoui_switch_create(struct picoui_window *parent, const char *id)
{
    struct picoui_switch *sw;

    if (parent == 0 || id == 0) {
        return 0;
    }

    sw = calloc(1, sizeof(*sw));
    if (sw == 0) {
        return 0;
    }

    sw->widget.backend_widget = picoui_backend_create_switch(parent->widget.backend_widget, id);
    if (sw->widget.backend_widget == 0) {
        free(sw);
        return 0;
    }

    sw->id = id;
    sw->widget.visible = 1;
    sw->widget.enabled = 1;
    if (picoui_backend_widget_bind_host(sw->widget.backend_widget, &sw->widget) != 0) {
        free(sw);
        return 0;
    }
    return sw;
}

struct picoui_switch *picoui_switch_create_with_props(struct picoui_window *parent,
                                                      const struct picoui_switch_props *props)
{
    struct picoui_switch *sw;
    struct picoui_backend_widget *backend;

    if (!picoui_switch_props_are_valid(props)) {
        return 0;
    }

    sw = picoui_switch_create(parent, props->id);
    if (sw == 0) {
        return 0;
    }

    backend = (struct picoui_backend_widget *)sw->widget.backend_widget;
    sw->checked = props->checked != 0;
    sw->cb = 0;
    sw->user_data = 0;
    if (picoui_backend_widget_update_value(backend,
                                           sw->checked,
                                           0,
                                           &sw->widget,
                                           0) != 0) {
        free(sw);
        return 0;
    }
    backend->last_signal = PICOUI_BACKEND_SIGNAL_NONE;
    backend->dispatch_count = 0;
    sw->cb = props->on_toggled;
    sw->user_data = props->user_data;
    if (picoui_widget_set_user_data(&sw->widget, props->user_data) != 0) {
        free(sw);
        return 0;
    }
    if (props->style_class != 0
        && picoui_widget_set_style_class(&sw->widget, props->style_class) != 0) {
        free(sw);
        return 0;
    }
    if ((props->width > 0 || props->height > 0)
        && picoui_widget_set_size(&sw->widget, props->width, props->height) != 0) {
        free(sw);
        return 0;
    }
    if (picoui_widget_set_bg_color(&sw->widget, props->bg_color) != 0
        || picoui_widget_set_text_color(&sw->widget, props->text_color) != 0
        || picoui_widget_set_border_color(&sw->widget, props->border_color) != 0
        || picoui_widget_set_radius(&sw->widget, props->radius) != 0
        || picoui_widget_set_padding(&sw->widget, props->padding) != 0) {
        free(sw);
        return 0;
    }
    return sw;
}

int picoui_switch_set_checked(struct picoui_switch *sw, int checked)
{
    int normalized_checked;

    if (sw == 0) {
        return -1;
    }

    normalized_checked = checked != 0;
    if (sw->checked == normalized_checked) {
        return 0;
    }

    if (sw->widget.backend_widget == 0) {
        return -1;
    }

    sw->checked = normalized_checked;
    return picoui_backend_widget_update_value(sw->widget.backend_widget,
                                              sw->checked,
                                              sw->cb,
                                              &sw->widget,
                                              sw->user_data);
}

int picoui_switch_is_checked(struct picoui_switch *sw)
{
    if (sw == 0) {
        return 0;
    }

    return sw->checked;
}

int picoui_switch_set_on_toggled(struct picoui_switch *sw,
                                 picoui_value_changed_cb cb,
                                 void *user_data)
{
    if (sw == 0) {
        return -1;
    }

    sw->cb = cb;
    sw->user_data = user_data;
    return 0;
}
