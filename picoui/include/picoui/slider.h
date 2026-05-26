#ifndef PICOUI_SLIDER_H
#define PICOUI_SLIDER_H

#include "picoui/widget.h"

struct picoui_window;
struct picoui_slider;

struct picoui_slider_props {
    const char *id;
    int min_value;
    int max_value;
    int value;
    picoui_value_changed_cb on_value_changed;
    void *user_data;
};

struct picoui_slider *picoui_slider_create(struct picoui_window *parent, const char *id);
struct picoui_slider *picoui_slider_create_with_props(struct picoui_window *parent,
                                                      const struct picoui_slider_props *props);
int picoui_slider_set_value(struct picoui_slider *slider, int value);
int picoui_slider_get_value(struct picoui_slider *slider);
int picoui_slider_set_range(struct picoui_slider *slider, int min_value, int max_value);
int picoui_slider_set_on_value_changed(struct picoui_slider *slider,
                                       picoui_value_changed_cb cb,
                                       void *user_data);

#endif
