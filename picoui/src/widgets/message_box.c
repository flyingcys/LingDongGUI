#include "internal.h"
#include "picoui/message_box.h"
#include "picoui/widget.h"

#include <stdlib.h>

int picoui_backend_message_box_set_title(struct picoui_message_box *box, const char *title);
int picoui_backend_message_box_set_message(struct picoui_message_box *box, const char *message);
int picoui_backend_message_box_set_confirm_text(struct picoui_message_box *box, const char *text);

static int picoui_message_box_props_are_valid(const struct picoui_message_box_props *props)
{
    return props != 0 && props->id != 0;
}

struct picoui_message_box *picoui_message_box_create(struct picoui_widget *parent, const char *id)
{
    struct picoui_message_box *box;

    if (parent == 0 || id == 0 || parent->backend_widget == 0) {
        return 0;
    }

    box = calloc(1, sizeof(*box));
    if (box == 0) {
        return 0;
    }

    box->widget.backend_widget = picoui_backend_create_message_box(parent->backend_widget, id);
    if (box->widget.backend_widget == 0) {
        free(box);
        return 0;
    }

    box->id = id;
    box->widget.visible = 1;
    box->widget.enabled = 1;
    if (picoui_backend_widget_bind_host(box->widget.backend_widget, &box->widget) != 0) {
        free(box);
        return 0;
    }
    return box;
}

struct picoui_message_box *picoui_message_box_create_with_props(
    struct picoui_widget *parent,
    const struct picoui_message_box_props *props)
{
    struct picoui_message_box *box;

    if (!picoui_message_box_props_are_valid(props)) {
        return 0;
    }

    box = picoui_message_box_create(parent, props->id);
    if (box == 0) {
        return 0;
    }

    if (props->style_class != 0
        && picoui_widget_set_style_class(&box->widget, props->style_class) != 0) {
        free(box);
        return 0;
    }
    if (picoui_widget_set_user_data(&box->widget, props->user_data) != 0) {
        free(box);
        return 0;
    }
    if ((props->title != 0 && picoui_message_box_set_title(box, props->title) != 0)
        || (props->message != 0 && picoui_message_box_set_message(box, props->message) != 0)
        || (props->confirm_text != 0
            && picoui_message_box_set_confirm_text(box, props->confirm_text) != 0)) {
        free(box);
        return 0;
    }

    return box;
}

int picoui_message_box_set_title(struct picoui_message_box *box, const char *title)
{
    if (box == 0 || title == 0) {
        return -1;
    }

    if (picoui_backend_message_box_set_title(box, title) != 0) {
        return -1;
    }

    box->title = title;
    return 0;
}

int picoui_message_box_set_message(struct picoui_message_box *box, const char *message)
{
    if (box == 0 || message == 0) {
        return -1;
    }

    if (picoui_backend_message_box_set_message(box, message) != 0) {
        return -1;
    }

    box->message = message;
    return 0;
}

int picoui_message_box_set_confirm_text(struct picoui_message_box *box, const char *text)
{
    if (box == 0 || text == 0) {
        return -1;
    }

    if (picoui_backend_message_box_set_confirm_text(box, text) != 0) {
        return -1;
    }

    box->confirm_text = text;
    return 0;
}

void picoui_message_box_set_on_confirm(
    struct picoui_message_box *box,
    picoui_message_box_callback_t callback,
    void *user_data)
{
    if (box == 0) {
        return;
    }

    box->on_confirm = callback;
    box->on_confirm_user_data = user_data;
    if (callback != 0) {
        (void)picoui_backend_message_box_set_on_confirm(box);
    }
}

const char *picoui_message_box_get_title(const struct picoui_message_box *box)
{
    if (box == 0) {
        return 0;
    }
    return box->title;
}

const char *picoui_message_box_get_message(const struct picoui_message_box *box)
{
    if (box == 0) {
        return 0;
    }
    return box->message;
}

const char *picoui_message_box_get_confirm_text(const struct picoui_message_box *box)
{
    if (box == 0) {
        return 0;
    }
    return box->confirm_text;
}
