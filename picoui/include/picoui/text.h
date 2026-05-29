#ifndef PICOUI_TEXT_H
#define PICOUI_TEXT_H

#include "picoui/widget.h"

struct picoui_window;
struct picoui_text;

struct picoui_text_props {
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

struct picoui_text *picoui_text_create(struct picoui_window *parent, const char *id);
struct picoui_text *picoui_text_create_with_props(struct picoui_window *parent,
                                                  const struct picoui_text_props *props);
int picoui_text_set_text(struct picoui_text *text, const char *value);
int picoui_text_set_font(struct picoui_text *text, const struct picoui_font *font);

#endif
