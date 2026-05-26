#include "internal.h"
#include "picoui/switch.h"

#include <stdlib.h>

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
    return sw;
}

struct picoui_switch *picoui_switch_create_with_props(struct picoui_window *parent,
                                                      const struct picoui_switch_props *props)
{
    struct picoui_switch *sw;

    if (props == 0) {
        return 0;
    }

    sw = picoui_switch_create(parent, props->id);
    if (sw == 0) {
        return 0;
    }

    sw->cb = props->on_toggled;
    sw->user_data = props->user_data;
    if (picoui_switch_set_checked(sw, props->checked) != 0) {
        free(sw);
        return 0;
    }

    return sw;
}

int picoui_switch_set_checked(struct picoui_switch *sw, int checked)
{
    if (sw == 0) {
        return -1;
    }

    sw->checked = checked != 0;
    picoui_backend_emit_value_changed(sw->cb, &sw->widget, sw->checked, sw->user_data);
    return 0;
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
