#ifndef PICOUI_LABEL_H
#define PICOUI_LABEL_H

#include "picoui/widget.h"

struct picoui_window;
struct picoui_label;

struct picoui_label *picoui_label_create(struct picoui_window *parent, const char *id);
int picoui_label_set_text(struct picoui_label *label, const char *text);
int picoui_label_set_font(struct picoui_label *label, const struct picoui_font *font);

#endif
