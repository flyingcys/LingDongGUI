#ifndef TINYUI_SCROLL_SELECTER_H
#define TINYUI_SCROLL_SELECTER_H

#include "core/widget.h"

struct tinyui_window;
struct tinyui_scroll_selecter;
struct tinyui_image_source;

struct tinyui_scroll_selecter_props {
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

struct tinyui_scroll_selecter *tinyui_scroll_selecter_create(struct tinyui_window *parent, const char *id);

struct tinyui_scroll_selecter *tinyui_scroll_selecter_create_with_props(
    struct tinyui_window *parent,
    const struct tinyui_scroll_selecter_props *props);

int tinyui_scroll_selecter_set_items(struct tinyui_scroll_selecter *scroll_selecter,
                                     const char *const *item_ids,
                                     const char *const *texts,
                                     int item_count);

int tinyui_scroll_selecter_add_item(struct tinyui_scroll_selecter *scroll_selecter,
                                    const char *id,
                                    const char *text);

int tinyui_scroll_selecter_set_select_item_num(struct tinyui_scroll_selecter *scroll_selecter, int index);

int tinyui_scroll_selecter_set_selected_index(struct tinyui_scroll_selecter *scroll_selecter, int index);

int tinyui_scroll_selecter_get_select_item_num(const struct tinyui_scroll_selecter *scroll_selecter);

int tinyui_scroll_selecter_get_selected_index(const struct tinyui_scroll_selecter *scroll_selecter);

const char *tinyui_scroll_selecter_get_select_text(const struct tinyui_scroll_selecter *scroll_selecter);

int tinyui_scroll_selecter_set_text_color(struct tinyui_scroll_selecter *scroll_selecter, unsigned int rgb);

int tinyui_scroll_selecter_set_background_color(struct tinyui_scroll_selecter *scroll_selecter, unsigned int rgb);

int tinyui_scroll_selecter_set_bg_color(struct tinyui_scroll_selecter *scroll_selecter, unsigned int rgb);

int tinyui_scroll_selecter_set_indicator_color(struct tinyui_scroll_selecter *scroll_selecter,
                                               unsigned int rgb);

int tinyui_scroll_selecter_set_background_image(struct tinyui_scroll_selecter *scroll_selecter,
                                                struct tinyui_image_source *source);

int tinyui_scroll_selecter_set_bg_source(struct tinyui_scroll_selecter *scroll_selecter,
                                         struct tinyui_image_source *source);

int tinyui_scroll_selecter_set_indicator_image(struct tinyui_scroll_selecter *scroll_selecter,
                                               struct tinyui_image_source *source);

int tinyui_scroll_selecter_set_indicator_source(struct tinyui_scroll_selecter *scroll_selecter,
                                                struct tinyui_image_source *source);

int tinyui_scroll_selecter_set_transparent(struct tinyui_scroll_selecter *scroll_selecter, int transparent);

int tinyui_scroll_selecter_set_speed(struct tinyui_scroll_selecter *scroll_selecter, int speed);

int tinyui_scroll_selecter_set_select_text(struct tinyui_scroll_selecter *scroll_selecter, const char *text);

int tinyui_scroll_selecter_set_edit_mode(struct tinyui_scroll_selecter *scroll_selecter, int is_edit);

int tinyui_scroll_selecter_get_edit_mode(const struct tinyui_scroll_selecter *scroll_selecter, int *is_edit);

const char *tinyui_scroll_selecter_get_selected_text(const struct tinyui_scroll_selecter *scroll_selecter);

#endif
