#ifndef PICOUI_LINE_EDIT_H
#define PICOUI_LINE_EDIT_H

#include "picoui/layout.h"
#include "picoui/widget.h"

struct picoui_window;
struct picoui_line_edit;
typedef void (*picoui_line_edit_finished_cb)(struct picoui_line_edit *line_edit, void *user_data);

enum picoui_line_edit_type {
    PICOUI_LINE_EDIT_TYPE_STRING = 0,
    PICOUI_LINE_EDIT_TYPE_INT,
    PICOUI_LINE_EDIT_TYPE_FLOAT,
};

struct picoui_line_edit_props {
    const char *id;
    const char *text;
    enum picoui_line_edit_type type;
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
    int has_type;
    int has_keyboard_binding;
};

struct picoui_line_edit *picoui_line_edit_create(struct picoui_window *parent, const char *id);
struct picoui_line_edit *picoui_line_edit_create_with_props(struct picoui_window *parent,
                                                            const struct picoui_line_edit_props *props);
int picoui_line_edit_set_text(struct picoui_line_edit *line_edit, const char *text);
const char *picoui_line_edit_get_text(const struct picoui_line_edit *line_edit);
int picoui_line_edit_set_align(struct picoui_line_edit *line_edit, enum picoui_align align);
int picoui_line_edit_set_color(struct picoui_line_edit *line_edit,
                               unsigned int text_color,
                               unsigned int background_color,
                               unsigned int frame_color);
int picoui_line_edit_set_type(struct picoui_line_edit *line_edit, enum picoui_line_edit_type type);
int picoui_line_edit_get_type(const struct picoui_line_edit *line_edit,
                              enum picoui_line_edit_type *type);
int picoui_line_edit_set_keyboard(struct picoui_line_edit *line_edit, unsigned int keyboard_binding);
int picoui_line_edit_set_keyboard_binding(struct picoui_line_edit *line_edit,
                                          unsigned int keyboard_binding);
int picoui_line_edit_get_keyboard_binding(const struct picoui_line_edit *line_edit,
                                          unsigned int *keyboard_binding);
int picoui_line_edit_get_editing(const struct picoui_line_edit *line_edit, int *editing);
int picoui_line_edit_set_on_edit_finished(struct picoui_line_edit *line_edit,
                                          picoui_line_edit_finished_cb cb,
                                          void *user_data);

#endif
