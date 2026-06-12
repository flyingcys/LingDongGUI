#ifndef TINYUI_IMAGE_H
#define TINYUI_IMAGE_H

#include "widget.h"

struct picoui_window;
struct picoui_image;

struct picoui_image_source {
    void *img_tile;
    void *mask_tile;
    unsigned int kind;
    unsigned int vres_addr;
};

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

int picoui_image_source_from_vres(unsigned int addr, struct picoui_image_source *out);

int picoui_font_from_vres(unsigned int addr, struct picoui_font *out);

void picoui_image_source_destroy(struct picoui_image_source *source);

void picoui_font_destroy(struct picoui_font *font);

int picoui_image_set_source(struct picoui_image *image, struct picoui_image_source *source);

int picoui_image_set_mask_color(struct picoui_image *image, unsigned int rgb);

#endif
