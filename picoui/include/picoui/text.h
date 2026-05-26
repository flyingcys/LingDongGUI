#ifndef PICOUI_TEXT_H
#define PICOUI_TEXT_H

#include "picoui/widget.h"

struct picoui_window;
struct picoui_text;

struct picoui_text *picoui_text_create(struct picoui_window *parent, const char *id);
int picoui_text_set_text(struct picoui_text *text, const char *value);
int picoui_text_set_font(struct picoui_text *text, const struct picoui_font *font);

#endif
