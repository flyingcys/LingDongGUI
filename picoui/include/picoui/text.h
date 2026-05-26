#ifndef PICOUI_TEXT_H
#define PICOUI_TEXT_H

struct picoui_window;
struct picoui_text;

struct picoui_text *picoui_text_create(struct picoui_window *parent, const char *id);
int picoui_text_set_text(struct picoui_text *text, const char *value);

#endif
