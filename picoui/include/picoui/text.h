#ifndef PICOUI_TEXT_H
#define PICOUI_TEXT_H

#include "picoui/widget.h"

struct picoui_window;
struct picoui_image_source;
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
int picoui_text_set_static_text(struct picoui_text *text, const char *value);
int picoui_text_set_font(struct picoui_text *text, const struct picoui_font *font);
int picoui_text_set_transparent(struct picoui_text *text, int transparent);
int picoui_text_set_text_color(struct picoui_text *text, unsigned int rgb);
int picoui_text_set_bg_color(struct picoui_text *text, unsigned int rgb);
int picoui_text_set_background_source(struct picoui_text *text,
                                      struct picoui_image_source *source);
int picoui_text_set_consumed_font(struct picoui_text *text, const struct picoui_font *font);
int picoui_text_scroll_seek(struct picoui_text *text, int offset);
int picoui_text_scroll_move(struct picoui_text *text, int move_value);

#endif
