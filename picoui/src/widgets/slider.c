#include "internal.h"
#include "picoui/slider.h"

#include <stdlib.h>

struct picoui_slider *picoui_slider_create(struct picoui_window *parent, const char *id)
{
    struct picoui_slider *slider;

    if (parent == 0 || id == 0) {
        return 0;
    }

    slider = calloc(1, sizeof(*slider));
    if (slider == 0) {
        return 0;
    }

    slider->widget.backend_widget = picoui_backend_create_slider(parent->widget.backend_widget, id);
    if (slider->widget.backend_widget == 0) {
        free(slider);
        return 0;
    }

    slider->id = id;
    slider->min_value = 0;
    slider->max_value = 100;
    slider->widget.visible = 1;
    slider->widget.enabled = 1;
    return slider;
}

struct picoui_slider *picoui_slider_create_with_props(struct picoui_window *parent,
                                                      const struct picoui_slider_props *props)
{
    struct picoui_slider *slider;

    if (props == 0) {
        return 0;
    }

    slider = picoui_slider_create(parent, props->id);
    if (slider == 0) {
        return 0;
    }

    if (props->min_value > props->max_value
        || props->value < props->min_value
        || props->value > props->max_value) {
        free(slider);
        return 0;
    }

    slider->min_value = props->min_value;
    slider->max_value = props->max_value;
    slider->value = props->value;
    slider->cb = props->on_value_changed;
    slider->user_data = props->user_data;
    ((struct picoui_backend_widget *)slider->widget.backend_widget)->value = slider->value;
    return slider;
}

int picoui_slider_set_value(struct picoui_slider *slider, int value)
{
    if (slider == 0 || value < slider->min_value || value > slider->max_value) {
        return -1;
    }

    if (slider->value == value) {
        return 0;
    }

    if (slider->widget.backend_widget == 0) {
        return -1;
    }

    slider->value = value;
    return picoui_backend_widget_dispatch_signal(slider->widget.backend_widget,
                                                 PICOUI_BACKEND_SIGNAL_VALUE_CHANGED,
                                                 slider->value,
                                                 slider->cb,
                                                 &slider->widget,
                                                 slider->user_data);
}

int picoui_slider_get_value(struct picoui_slider *slider)
{
    if (slider == 0) {
        return 0;
    }

    return slider->value;
}

int picoui_slider_set_range(struct picoui_slider *slider, int min_value, int max_value)
{
    if (slider == 0 || min_value > max_value) {
        return -1;
    }

    slider->min_value = min_value;
    slider->max_value = max_value;
    if (slider->value < min_value) {
        slider->value = min_value;
    }
    if (slider->value > max_value) {
        slider->value = max_value;
    }
    return 0;
}

int picoui_slider_set_on_value_changed(struct picoui_slider *slider,
                                       picoui_value_changed_cb cb,
                                       void *user_data)
{
    if (slider == 0) {
        return -1;
    }

    slider->cb = cb;
    slider->user_data = user_data;
    return 0;
}
