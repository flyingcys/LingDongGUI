#ifndef TINYUI_LINE_EDIT_H
#define TINYUI_LINE_EDIT_H

#include "layout/layout.h"
#include "core/widget.h"

struct tinyui_window;
struct tinyui_line_edit;
struct tinyui_keyboard;
typedef void (*tinyui_line_edit_finished_cb)(struct tinyui_line_edit *line_edit, void *user_data);

enum tinyui_line_edit_type {
    TINYUI_LINE_EDIT_TYPE_STRING = 0,
    TINYUI_LINE_EDIT_TYPE_INT,
    TINYUI_LINE_EDIT_TYPE_FLOAT,
};

struct tinyui_line_edit_props {
    const char *id;
    const char *text;
    /* Sentinel default: -1 = unset (use backend default type) */
    enum tinyui_line_edit_type type;
    /* Sentinel default: 0 = unset (no keyboard binding applied) */
    unsigned int keyboard_binding;
    const char *style_class;
    void *user_data;
    int width;
    int height;
    unsigned int bg_color;
    unsigned int text_color;
    unsigned int border_color;
    int radius;
    int padding;
};

struct tinyui_line_edit *tinyui_line_edit_create(struct tinyui_window *parent, const char *id);

struct tinyui_line_edit *tinyui_line_edit_create_with_props(
    struct tinyui_window *parent,
    const struct tinyui_line_edit_props *props);

int tinyui_line_edit_set_text(struct tinyui_line_edit *line_edit, const char *text);

const char *tinyui_line_edit_get_text(const struct tinyui_line_edit *line_edit);

int tinyui_line_edit_set_align(struct tinyui_line_edit *line_edit, enum tinyui_align align);

int tinyui_line_edit_set_color(struct tinyui_line_edit *line_edit,
                               unsigned int text_color,
                               unsigned int background_color,
                               unsigned int frame_color);

int tinyui_line_edit_set_type(struct tinyui_line_edit *line_edit, enum tinyui_line_edit_type type);

int tinyui_line_edit_get_type(const struct tinyui_line_edit *line_edit,
                              enum tinyui_line_edit_type *type);

int tinyui_line_edit_set_keyboard(struct tinyui_line_edit *line_edit, unsigned int keyboard_binding);

int tinyui_line_edit_set_keyboard_binding(struct tinyui_line_edit *line_edit,
                                          unsigned int keyboard_binding);

int tinyui_line_edit_set_keyboard_widget(struct tinyui_line_edit *line_edit,
                                         struct tinyui_keyboard *keyboard);

int tinyui_line_edit_get_keyboard_binding(const struct tinyui_line_edit *line_edit,
                                          unsigned int *keyboard_binding);

int tinyui_line_edit_get_editing(const struct tinyui_line_edit *line_edit, int *editing);

int tinyui_line_edit_set_on_edit_finished(struct tinyui_line_edit *line_edit,
                                          tinyui_line_edit_finished_cb cb,
                                          void *user_data);

#endif
