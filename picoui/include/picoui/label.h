#ifndef PICOUI_LABEL_H
#define PICOUI_LABEL_H

#include "picoui/widget.h"

struct picoui_window;
struct picoui_label;
struct picoui_image_source;

struct picoui_label_props {
    const char *id;
    const char *text;
    const struct picoui_font *font;
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

struct picoui_label *picoui_label_create(struct picoui_window *parent, const char *id);
struct picoui_label *picoui_label_create_with_props(struct picoui_window *parent,
                                                    const struct picoui_label_props *props);
int picoui_label_set_text(struct picoui_label *label, const char *text);
const char *picoui_label_get_text(struct picoui_label *label);
int picoui_label_set_font(struct picoui_label *label, const struct picoui_font *font);
int picoui_label_set_text_color(struct picoui_label *label, unsigned int rgb);
int picoui_label_get_text_color(struct picoui_label *label, unsigned int *rgb);
int picoui_label_set_bg_color(struct picoui_label *label, unsigned int rgb);
int picoui_label_get_bg_color(struct picoui_label *label, unsigned int *rgb);
int picoui_label_set_transparent(struct picoui_label *label, int transparent);
int picoui_label_get_transparent(struct picoui_label *label, int *transparent);
int picoui_label_set_align(struct picoui_label *label, enum picoui_align align);
int picoui_label_get_align(struct picoui_label *label, enum picoui_align *align);
int picoui_label_set_background_source(struct picoui_label *label,
                                       struct picoui_image_source *source);

#endif
