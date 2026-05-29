#ifndef PICOUI_LABEL_H
#define PICOUI_LABEL_H

#include "picoui/widget.h"

struct picoui_window;
struct picoui_label;

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
int picoui_label_set_font(struct picoui_label *label, const struct picoui_font *font);

#endif
