#ifndef PICOUI_BUTTON_H
#define PICOUI_BUTTON_H

#include "picoui/widget.h"

struct picoui_window;
struct picoui_button;

struct picoui_button_props {
    const char *id;
    const char *text;
    int width;
    int height;
    picoui_event_cb on_clicked;
    void *user_data;
};

struct picoui_button *picoui_button_create(struct picoui_window *parent, const char *id);
struct picoui_button *picoui_button_create_with_props(struct picoui_window *parent,
                                                      const struct picoui_button_props *props);
int picoui_button_set_text(struct picoui_button *button, const char *text);

#endif
