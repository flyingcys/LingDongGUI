#include "internal.h"
#include "picoui/slider.h"

#include <stdlib.h>

static int picoui_slider_props_are_valid(const struct picoui_slider_props *props)
{
    return props != 0
        && props->id != 0
        && props->min_value <= props->max_value
        && props->value >= props->min_value
        && props->value <= props->max_value
        && props->width >= 0
        && props->height >= 0
        && props->radius >= 0
        && props->padding >= 0;
}

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
    if (picoui_backend_widget_bind_host(slider->widget.backend_widget, &slider->widget) != 0) {
        free(slider);
        return 0;
    }
    return slider;
}

struct picoui_slider *picoui_slider_create_with_props(struct picoui_window *parent,
                                                      const struct picoui_slider_props *props)
{
    struct picoui_slider *slider;
    struct picoui_backend_widget *backend;

    if (!picoui_slider_props_are_valid(props)) {
        return 0;
    }

    slider = picoui_slider_create(parent, props->id);
    if (slider == 0) {
        return 0;
    }

    slider->min_value = props->min_value;
    slider->max_value = props->max_value;
    slider->value = props->value;
    slider->cb = 0;
    slider->user_data = 0;
    backend = (struct picoui_backend_widget *)slider->widget.backend_widget;
    if (picoui_backend_widget_update_value(backend,
                                           slider->value,
                                           0,
                                           &slider->widget,
                                           0) != 0) {
        free(slider);
        return 0;
    }
    backend->last_signal = PICOUI_BACKEND_SIGNAL_NONE;
    backend->dispatch_count = 0;
    slider->cb = props->on_value_changed;
    slider->user_data = props->user_data;
    if (picoui_widget_set_user_data(&slider->widget, props->user_data) != 0) {
        free(slider);
        return 0;
    }
    if (props->style_class != 0
        && picoui_widget_set_style_class(&slider->widget, props->style_class) != 0) {
        free(slider);
        return 0;
    }
    if ((props->width > 0 || props->height > 0)
        && picoui_widget_set_size(&slider->widget, props->width, props->height) != 0) {
        free(slider);
        return 0;
    }
    if (picoui_widget_set_bg_color(&slider->widget, props->bg_color) != 0
        || picoui_widget_set_text_color(&slider->widget, props->text_color) != 0
        || picoui_widget_set_border_color(&slider->widget, props->border_color) != 0
        || picoui_widget_set_radius(&slider->widget, props->radius) != 0
        || picoui_widget_set_padding(&slider->widget, props->padding) != 0) {
        free(slider);
        return 0;
    }
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
    return picoui_backend_widget_update_value(slider->widget.backend_widget,
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
    int clamped_value;

    if (slider == 0 || min_value > max_value) {
        return -1;
    }

    slider->min_value = min_value;
    slider->max_value = max_value;
    clamped_value = slider->value;
    if (clamped_value < min_value) {
        clamped_value = min_value;
    }
    if (clamped_value > max_value) {
        clamped_value = max_value;
    }

    slider->value = clamped_value;
    if (slider->widget.backend_widget != 0) {
        return picoui_backend_widget_update_value(slider->widget.backend_widget,
                                                  slider->value,
                                                  0,
                                                  &slider->widget,
                                                  0);
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
