#ifndef TINYUI_SCROLL_SELECTER_H
#define TINYUI_SCROLL_SELECTER_H

#include "widget.h"

struct picoui_window;
struct picoui_scroll_selecter;
struct picoui_image_source;

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
    const struct picoui_scroll_selecter_props *props);

int picoui_scroll_selecter_set_items(struct picoui_scroll_selecter *scroll_selecter,
                                     const char *const *item_ids,
                                     const char *const *texts,
                                     int item_count);

int picoui_scroll_selecter_add_item(struct picoui_scroll_selecter *scroll_selecter,
                                    const char *id,
                                    const char *text);

int picoui_scroll_selecter_set_select_item_num(struct picoui_scroll_selecter *scroll_selecter, int index);

int picoui_scroll_selecter_set_selected_index(struct picoui_scroll_selecter *scroll_selecter, int index);

int picoui_scroll_selecter_get_select_item_num(const struct picoui_scroll_selecter *scroll_selecter);

int picoui_scroll_selecter_get_selected_index(const struct picoui_scroll_selecter *scroll_selecter);

const char *picoui_scroll_selecter_get_select_text(const struct picoui_scroll_selecter *scroll_selecter);

int picoui_scroll_selecter_set_text_color(struct picoui_scroll_selecter *scroll_selecter, unsigned int rgb);

int picoui_scroll_selecter_set_background_color(struct picoui_scroll_selecter *scroll_selecter, unsigned int rgb);

int picoui_scroll_selecter_set_bg_color(struct picoui_scroll_selecter *scroll_selecter, unsigned int rgb);

int picoui_scroll_selecter_set_indicator_color(struct picoui_scroll_selecter *scroll_selecter,
                                               unsigned int rgb);

int picoui_scroll_selecter_set_background_image(struct picoui_scroll_selecter *scroll_selecter,
                                                struct picoui_image_source *source);

int picoui_scroll_selecter_set_bg_source(struct picoui_scroll_selecter *scroll_selecter,
                                         struct picoui_image_source *source);

int picoui_scroll_selecter_set_indicator_image(struct picoui_scroll_selecter *scroll_selecter,
                                               struct picoui_image_source *source);

int picoui_scroll_selecter_set_indicator_source(struct picoui_scroll_selecter *scroll_selecter,
                                                struct picoui_image_source *source);

int picoui_scroll_selecter_set_transparent(struct picoui_scroll_selecter *scroll_selecter, int transparent);

int picoui_scroll_selecter_set_speed(struct picoui_scroll_selecter *scroll_selecter, int speed);

int picoui_scroll_selecter_set_select_text(struct picoui_scroll_selecter *scroll_selecter, const char *text);

int picoui_scroll_selecter_set_edit_mode(struct picoui_scroll_selecter *scroll_selecter, int is_edit);

int picoui_scroll_selecter_get_edit_mode(const struct picoui_scroll_selecter *scroll_selecter, int *is_edit);

const char *picoui_scroll_selecter_get_selected_text(const struct picoui_scroll_selecter *scroll_selecter);

#endif
