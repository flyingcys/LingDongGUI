#ifndef PICOUI_IMAGE_H
#define PICOUI_IMAGE_H

#include "picoui/widget.h"

struct picoui_window;
struct picoui_image;
struct picoui_image_source;

struct picoui_image_props {
    const char *id;
    struct picoui_image_source *source;
    const char *style_class;
    void *user_data;
    int width;
    int height;
    unsigned int bg_color;
    unsigned int text_color;
    unsigned int border_color;
    int radius;
    int padding;
};

struct picoui_image *picoui_image_create(struct picoui_window *parent, const char *id);
struct picoui_image *picoui_image_create_with_props(struct picoui_window *parent,
                                                    const struct picoui_image_props *props);
int picoui_image_set_source(struct picoui_image *image, struct picoui_image_source *source);

#endif
