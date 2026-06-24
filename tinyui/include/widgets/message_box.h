#ifndef TINYUI_MESSAGE_BOX_H
#define TINYUI_MESSAGE_BOX_H

#include "core/widget.h"

struct tinyui_message_box;

typedef void (*tinyui_message_box_callback_t)(struct tinyui_message_box *box, void *user_data);
typedef void (*tinyui_message_box_indexed_callback_t)(struct tinyui_message_box *box,
                                                      int index,
                                                      void *user_data);

struct tinyui_message_box_props {
    const char *id;
    const char *style_class;
    void *user_data;
    const char *title;
    const char *message;
    const char *confirm_text;
};

struct tinyui_message_box *tinyui_message_box_create(struct tinyui_widget *parent, const char *id);

struct tinyui_message_box *tinyui_message_box_init(struct tinyui_widget *parent, const char *id);

struct tinyui_message_box *tinyui_message_box_create_with_props(
    struct tinyui_widget *parent,
    const struct tinyui_message_box_props *props);

int tinyui_message_box_set_title(struct tinyui_message_box *box, const char *title);

int tinyui_message_box_set_message(struct tinyui_message_box *box, const char *message);

int tinyui_message_box_set_msg(struct tinyui_message_box *box, const char *message);

int tinyui_message_box_set_confirm_text(struct tinyui_message_box *box, const char *text);

int tinyui_message_box_set_buttons(struct tinyui_message_box *box, const char *const *buttons, int count);

int tinyui_message_box_set_btn(struct tinyui_message_box *box, const char *const *buttons, int count);

int tinyui_message_box_set_string_colors(struct tinyui_message_box *box,
                                         unsigned int title_color,
                                         unsigned int message_color,
                                         unsigned int button_color);

int tinyui_message_box_set_string_color(struct tinyui_message_box *box,
                                        unsigned int title_color,
                                        unsigned int message_color,
                                        unsigned int button_color);

int tinyui_message_box_set_button_colors(struct tinyui_message_box *box,
                                         unsigned int release_color,
                                         unsigned int press_color);

int tinyui_message_box_set_button_color(struct tinyui_message_box *box,
                                        unsigned int release_color,
                                        unsigned int press_color);

int tinyui_message_box_set_bg_color(struct tinyui_message_box *box, unsigned int bg_color);

int tinyui_message_box_set_background_color(struct tinyui_message_box *box, unsigned int bg_color);

void tinyui_message_box_set_on_confirm(struct tinyui_message_box *box,
                                       tinyui_message_box_callback_t callback,
                                       void *user_data);

void tinyui_message_box_set_callback(struct tinyui_message_box *box,
                                     tinyui_message_box_callback_t callback,
                                     void *user_data);

void tinyui_message_box_set_on_confirm_indexed(struct tinyui_message_box *box,
                                               tinyui_message_box_indexed_callback_t callback,
                                               void *user_data);

const char *tinyui_message_box_get_title(const struct tinyui_message_box *box);

const char *tinyui_message_box_get_message(const struct tinyui_message_box *box);

const char *tinyui_message_box_get_confirm_text(const struct tinyui_message_box *box);

#endif
