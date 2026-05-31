#ifndef PICOUI_SLIDER_H
#define PICOUI_SLIDER_H

#include "picoui/widget.h"

struct picoui_window;
struct picoui_slider;
struct picoui_image_source;

struct picoui_slider_props {
    const char *id;
    int min_value;
    int max_value;
    int value;
    picoui_value_changed_cb on_value_changed;
    void *user_data;
    const char *style_class;
    int width;
    int height;
    unsigned int bg_color;
    unsigned int text_color;
    unsigned int border_color;
    int radius;
    int padding;

    int horizontal;
    struct picoui_image_source *background_source;
    struct picoui_image_source *indicator_source;
    int indicator_width;
    int slim_size;

    int has_horizontal;
    int has_background_source;
    int has_indicator_source;
    int has_indicator_width;
    int has_slim_size;
};

struct picoui_slider *picoui_slider_create(struct picoui_window *parent, const char *id);
struct picoui_slider *picoui_slider_create_with_props(struct picoui_window *parent,
                                                      const struct picoui_slider_props *props);
int picoui_slider_set_value(struct picoui_slider *slider, int value);
int picoui_slider_set_range(struct picoui_slider *slider, int min_value, int max_value);
int picoui_slider_set_horizontal(struct picoui_slider *slider, int horizontal);
int picoui_slider_get_horizontal(struct picoui_slider *slider, int *horizontal);
int picoui_slider_set_background_source(struct picoui_slider *slider,
                                        struct picoui_image_source *source);
int picoui_slider_set_indicator_source(struct picoui_slider *slider,
                                       struct picoui_image_source *source);
int picoui_slider_set_indicator_width(struct picoui_slider *slider, int indicator_width);
int picoui_slider_set_slim_size(struct picoui_slider *slider, int slim_size);
int picoui_slider_get_percent(struct picoui_slider *slider, int *percent);
int picoui_slider_set_on_value_changed(struct picoui_slider *slider,
                                       picoui_value_changed_cb cb,
                                       void *user_data);

#endif
