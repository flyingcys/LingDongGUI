#ifndef TINYUI_IMAGE_H
#define TINYUI_IMAGE_H

#include "resource/font.h"
#include "resource/image_source.h"

struct tinyui_window;
struct tinyui_image;

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

int tinyui_image_set_source(struct tinyui_image *image,
                            struct tinyui_image_source *source);

int tinyui_image_set_mask_color(struct tinyui_image *image, unsigned int rgb);

#endif
