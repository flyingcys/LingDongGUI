#ifndef PICOUI_CHECKBOX_H
#define PICOUI_CHECKBOX_H

#include "picoui/widget.h"

struct picoui_window;
struct picoui_checkbox;

struct picoui_checkbox_props {
    const char *id;
    const char *text;
    int checked;
    picoui_value_changed_cb on_toggled;
    void *user_data;
    const char *style_class;
    int width;
    int height;
    unsigned int bg_color;
    unsigned int text_color;
    unsigned int border_color;
    int radius;
    int padding;
};

struct picoui_checkbox *picoui_checkbox_create(struct picoui_window *parent, const char *id);
struct picoui_checkbox *picoui_checkbox_create_with_props(struct picoui_window *parent,
                                                          const struct picoui_checkbox_props *props);
int picoui_checkbox_set_checked(struct picoui_checkbox *checkbox, int checked);
int picoui_checkbox_is_checked(struct picoui_checkbox *checkbox);
int picoui_checkbox_set_text(struct picoui_checkbox *checkbox, const char *text);
int picoui_checkbox_set_on_toggled(struct picoui_checkbox *checkbox,
                                   picoui_value_changed_cb cb,
                                   void *user_data);

#endif
