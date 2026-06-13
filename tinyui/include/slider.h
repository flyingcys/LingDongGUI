#ifndef TINYUI_SLIDER_H
#define TINYUI_SLIDER_H

#include "widget.h"

struct tinyui_window;
struct tinyui_slider;
struct tinyui_image_source;

struct tinyui_slider_props {
    const char *id;
    int min_value;
    int max_value;
    int value;
    tinyui_value_changed_cb on_value_changed;
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
    struct tinyui_image_source *background_source;
    struct tinyui_image_source *indicator_source;
    int indicator_width;
    int slim_size;

    int has_horizontal;
    int has_background_source;
    int has_indicator_source;
    int has_indicator_width;
    int has_slim_size;
};

struct tinyui_slider *tinyui_slider_create(struct tinyui_window *parent, const char *id);

struct tinyui_slider *tinyui_slider_create_with_props(
    struct tinyui_window *parent,
    const struct tinyui_slider_props *props);

struct tinyui_slider *tinyui_slider_init(struct tinyui_window *parent, const char *id);

int tinyui_slider_set_value(struct tinyui_slider *slider, int value);

int tinyui_slider_set_range(struct tinyui_slider *slider, int min_value, int max_value);

int tinyui_slider_set_percent(struct tinyui_slider *slider, int percent);

int tinyui_slider_set_horizontal(struct tinyui_slider *slider, int horizontal);

int tinyui_slider_get_horizontal(struct tinyui_slider *slider, int *horizontal);

int tinyui_slider_set_background_source(struct tinyui_slider *slider,
                                        struct tinyui_image_source *source);

int tinyui_slider_set_indicator_source(struct tinyui_slider *slider,
                                       struct tinyui_image_source *source);

int tinyui_slider_set_image(struct tinyui_slider *slider,
                            struct tinyui_image_source *background_source,
                            struct tinyui_image_source *indicator_source);

int tinyui_slider_set_color(struct tinyui_slider *slider,
                            unsigned int bg_color,
                            unsigned int frame_color,
                            unsigned int indicator_color);

int tinyui_slider_set_indicator_width(struct tinyui_slider *slider, int indicator_width);

int tinyui_slider_set_slim_size(struct tinyui_slider *slider, int slim_size);

int tinyui_slider_get_percent(struct tinyui_slider *slider, int *percent);

int tinyui_slider_set_on_value_changed(struct tinyui_slider *slider,
                                       tinyui_value_changed_cb cb,
                                       void *user_data);

#endif
