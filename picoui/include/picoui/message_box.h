#ifndef PICOUI_MESSAGE_BOX_H
#define PICOUI_MESSAGE_BOX_H

#include "picoui/widget.h"

struct picoui_message_box;

typedef void (*picoui_message_box_callback_t)(struct picoui_message_box *box, void *user_data);

struct picoui_message_box_props {
    const char *id;
    const char *style_class;
    void *user_data;
    const char *title;
    const char *message;
    const char *confirm_text;
};

struct picoui_message_box *picoui_message_box_create(struct picoui_widget *parent, const char *id);
struct picoui_message_box *picoui_message_box_create_with_props(
    struct picoui_widget *parent,
    const struct picoui_message_box_props *props);
int picoui_message_box_set_title(struct picoui_message_box *box, const char *title);
int picoui_message_box_set_message(struct picoui_message_box *box, const char *message);
int picoui_message_box_set_confirm_text(struct picoui_message_box *box, const char *text);
void picoui_message_box_set_on_confirm(
    struct picoui_message_box *box,
    picoui_message_box_callback_t callback,
    void *user_data);
const char *picoui_message_box_get_title(const struct picoui_message_box *box);
const char *picoui_message_box_get_message(const struct picoui_message_box *box);
const char *picoui_message_box_get_confirm_text(const struct picoui_message_box *box);

#endif
