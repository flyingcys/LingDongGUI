#ifndef PICOUI_SWITCH_H
#define PICOUI_SWITCH_H

#include "picoui/widget.h"

struct picoui_window;
struct picoui_switch;

struct picoui_switch_props {
    const char *id;
    int checked;
    picoui_value_changed_cb on_toggled;
    void *user_data;
};

struct picoui_switch *picoui_switch_create(struct picoui_window *parent, const char *id);
struct picoui_switch *picoui_switch_create_with_props(struct picoui_window *parent,
                                                      const struct picoui_switch_props *props);
int picoui_switch_set_checked(struct picoui_switch *sw, int checked);
int picoui_switch_is_checked(struct picoui_switch *sw);
int picoui_switch_set_on_toggled(struct picoui_switch *sw,
                                 picoui_value_changed_cb cb,
                                 void *user_data);

#endif
