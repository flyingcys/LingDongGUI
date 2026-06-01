#ifndef PICOUI_SWITCH_H
#define PICOUI_SWITCH_H

#include "picoui/widget.h"

struct picoui_window;
struct picoui_switch;
struct picoui_image_source;

struct picoui_switch_props {
    const char *id;
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
    struct picoui_image_source *off_source;
    struct picoui_image_source *on_source;
    struct picoui_image_source *knob_source;
    int horizontal;
    int direction;
    int disabled;

    int has_off_source;
    int has_on_source;
    int has_knob_source;
    int has_horizontal;
    int has_direction;
    int has_disabled;
};

struct picoui_switch *picoui_switch_create(struct picoui_window *parent, const char *id);
struct picoui_switch *picoui_switch_create_with_props(struct picoui_window *parent,
                                                      const struct picoui_switch_props *props);
int picoui_switch_set_checked(struct picoui_switch *sw, int checked);
int picoui_switch_is_checked(struct picoui_switch *sw);
int picoui_switch_set_off_source(struct picoui_switch *sw, struct picoui_image_source *source);
int picoui_switch_set_on_source(struct picoui_switch *sw, struct picoui_image_source *source);
int picoui_switch_set_knob_source(struct picoui_switch *sw, struct picoui_image_source *source);
int picoui_switch_set_horizontal(struct picoui_switch *sw, int horizontal);
int picoui_switch_get_horizontal(struct picoui_switch *sw, int *horizontal);
int picoui_switch_set_direction(struct picoui_switch *sw, int direction);
int picoui_switch_get_direction(struct picoui_switch *sw, int *direction);
int picoui_switch_set_disabled(struct picoui_switch *sw, int disabled);
int picoui_switch_get_disabled(struct picoui_switch *sw, int *disabled);
int picoui_switch_can_navigate(struct picoui_switch *sw, int direction, int *can_navigate);
int picoui_switch_navigate(struct picoui_switch *sw, int direction);
int picoui_switch_set_on_toggled(struct picoui_switch *sw,
                                 picoui_value_changed_cb cb,
                                 void *user_data);

#endif
