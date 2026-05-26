#ifndef PICOUI_LABEL_H
#define PICOUI_LABEL_H

struct picoui_window;
struct picoui_label;

struct picoui_label *picoui_label_create(struct picoui_window *parent, const char *id);
int picoui_label_set_text(struct picoui_label *label, const char *text);

#endif
