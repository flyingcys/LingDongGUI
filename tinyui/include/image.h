#ifndef TINYUI_IMAGE_H
#define TINYUI_IMAGE_H

#include "widget.h"

struct tinyui_window;
struct tinyui_image;

struct tinyui_image_source {
    void *img_tile;
    void *mask_tile;
    unsigned int kind;
    unsigned int vres_addr;
};

struct tinyui_image_props {
    const char *id;
    struct tinyui_image_source *source;
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

struct tinyui_image *tinyui_image_create(struct tinyui_window *parent, const char *id);

struct tinyui_image *tinyui_image_create_with_props(struct tinyui_window *parent,
                                                    const struct tinyui_image_props *props);

int tinyui_image_source_from_vres(unsigned int addr, struct tinyui_image_source *out);

int tinyui_font_from_vres(unsigned int addr, struct tinyui_font *out);

void tinyui_image_source_destroy(struct tinyui_image_source *source);

void tinyui_font_destroy(struct tinyui_font *font);

int tinyui_image_set_source(struct tinyui_image *image, struct tinyui_image_source *source);

int tinyui_image_set_mask_color(struct tinyui_image *image, unsigned int rgb);

#endif
