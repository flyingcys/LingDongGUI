#include "internal.h"
#include "backend.h"
#include "picoui/message_box.h"
#include "picoui/widget.h"

#include <stdlib.h>

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

struct picoui_message_box *picoui_message_box_init(struct picoui_widget *parent, const char *id)
{
    return picoui_message_box_create(parent, id);
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

int picoui_message_box_set_msg(struct picoui_message_box *box, const char *message)
{
    return picoui_message_box_set_message(box, message);
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

int picoui_message_box_set_buttons(struct picoui_message_box *box, const char *const *buttons, int count)
{
    int i;

    if (box == 0 || buttons == 0 || count <= 0 || count > PICOUI_LIST_MAX_ITEMS) {
        return -1;
    }
    for (i = 0; i < count; ++i) {
        if (buttons[i] == 0) {
            return -1;
        }
    }

    if (picoui_backend_message_box_set_buttons(box, buttons, count) != 0) {
        return -1;
    }

    for (i = 0; i < count; ++i) {
        box->buttons[i] = buttons[i];
    }
    box->button_count = count;
    if (count > 0) {
        box->confirm_text = buttons[count - 1];
    }
    return 0;
}

int picoui_message_box_set_btn(struct picoui_message_box *box, const char *const *buttons, int count)
{
    return picoui_message_box_set_buttons(box, buttons, count);
}

int picoui_message_box_set_string_colors(struct picoui_message_box *box,
                                         unsigned int title_color,
                                         unsigned int message_color,
                                         unsigned int button_color)
{
    if (box == 0) {
        return -1;
    }

    if (picoui_backend_message_box_set_string_colors(box, title_color, message_color, button_color) != 0) {
        return -1;
    }

    box->title_color = title_color;
    box->message_color = message_color;
    box->button_color = button_color;
    return 0;
}

int picoui_message_box_set_string_color(struct picoui_message_box *box,
                                        unsigned int title_color,
                                        unsigned int message_color,
                                        unsigned int button_color)
{
    return picoui_message_box_set_string_colors(box, title_color, message_color, button_color);
}

int picoui_message_box_set_button_colors(struct picoui_message_box *box,
                                         unsigned int release_color,
                                         unsigned int press_color)
{
    if (box == 0) {
        return -1;
    }

    if (picoui_backend_message_box_set_button_colors(box, release_color, press_color) != 0) {
        return -1;
    }

    box->release_color = release_color;
    box->press_color = press_color;
    return 0;
}

int picoui_message_box_set_button_color(struct picoui_message_box *box,
                                        unsigned int release_color,
                                        unsigned int press_color)
{
    return picoui_message_box_set_button_colors(box, release_color, press_color);
}

int picoui_message_box_set_bg_color(struct picoui_message_box *box, unsigned int bg_color)
{
    if (box == 0) {
        return -1;
    }

    if (picoui_backend_message_box_set_bg_color(box, bg_color) != 0) {
        return -1;
    }

    box->bg_color = bg_color;
    return 0;
}

int picoui_message_box_set_background_color(struct picoui_message_box *box, unsigned int bg_color)
{
    return picoui_message_box_set_bg_color(box, bg_color);
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

void picoui_message_box_set_callback(
    struct picoui_message_box *box,
    picoui_message_box_callback_t callback,
    void *user_data)
{
    picoui_message_box_set_on_confirm(box, callback, user_data);
}

void picoui_message_box_set_on_confirm_indexed(
    struct picoui_message_box *box,
    picoui_message_box_indexed_callback_t callback,
    void *user_data)
{
    if (box == 0) {
        return;
    }

    box->on_confirm_indexed = callback;
    box->on_confirm_indexed_user_data = user_data;
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
