#ifndef PICOUI_CHECKBOX_H
#define PICOUI_CHECKBOX_H

#include "picoui/widget.h"

struct picoui_window;
struct picoui_checkbox;
struct picoui_image_source;

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
    unsigned int check_color;
    struct picoui_image_source *unchecked_source;
    struct picoui_image_source *checked_source;
    int radio_group;
    int string_left_space;

    int has_check_color;
    int has_unchecked_source;
    int has_checked_source;
    int has_radio_group;
    int has_string_left_space;
};

struct picoui_checkbox *picoui_checkbox_create(struct picoui_window *parent, const char *id);
struct picoui_checkbox *picoui_checkbox_create_with_props(struct picoui_window *parent,
                                                          const struct picoui_checkbox_props *props);
int picoui_checkbox_set_checked(struct picoui_checkbox *checkbox, int checked);
int picoui_checkbox_is_checked(struct picoui_checkbox *checkbox);
int picoui_checkbox_set_text(struct picoui_checkbox *checkbox, const char *text);
int picoui_checkbox_set_check_color(struct picoui_checkbox *checkbox, unsigned int rgb);
int picoui_checkbox_set_text_color(struct picoui_checkbox *checkbox, unsigned int rgb);
int picoui_checkbox_set_unchecked_source(struct picoui_checkbox *checkbox,
                                         struct picoui_image_source *source);
int picoui_checkbox_set_checked_source(struct picoui_checkbox *checkbox,
                                       struct picoui_image_source *source);
int picoui_checkbox_set_radio_group(struct picoui_checkbox *checkbox, int radio_group);
int picoui_checkbox_set_string_left_space(struct picoui_checkbox *checkbox, int space);
int picoui_checkbox_set_on_toggled(struct picoui_checkbox *checkbox,
                                   picoui_value_changed_cb cb,
                                   void *user_data);

#endif
