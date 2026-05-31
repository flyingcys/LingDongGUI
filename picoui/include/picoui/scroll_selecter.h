#ifndef PICOUI_SCROLL_SELECTER_H
#define PICOUI_SCROLL_SELECTER_H

#include "picoui/widget.h"

struct picoui_window;
struct picoui_scroll_selecter;

struct picoui_scroll_selecter_props {
    const char *id;
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

struct picoui_scroll_selecter *picoui_scroll_selecter_create(struct picoui_window *parent, const char *id);
struct picoui_scroll_selecter *picoui_scroll_selecter_create_with_props(
    struct picoui_window *parent,
    const struct picoui_scroll_selecter_props *props
);
int picoui_scroll_selecter_add_item(struct picoui_scroll_selecter *scroll_selecter,
                                    const char *id,
                                    const char *text);
int picoui_scroll_selecter_set_selected_index(struct picoui_scroll_selecter *scroll_selecter, int index);
int picoui_scroll_selecter_get_selected_index(const struct picoui_scroll_selecter *scroll_selecter);
int picoui_scroll_selecter_set_edit_mode(struct picoui_scroll_selecter *scroll_selecter, int is_edit);
int picoui_scroll_selecter_get_edit_mode(const struct picoui_scroll_selecter *scroll_selecter, int *is_edit);

#endif
