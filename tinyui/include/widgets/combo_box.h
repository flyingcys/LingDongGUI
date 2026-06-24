#ifndef TINYUI_COMBO_BOX_H
#define TINYUI_COMBO_BOX_H

#include "core/widget.h"

struct tinyui_window;
struct tinyui_combo_box;
struct tinyui_image_source;

struct tinyui_combo_box_props {
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

struct tinyui_combo_box *tinyui_combo_box_create(struct tinyui_window *parent, const char *id);

struct tinyui_combo_box *tinyui_combo_box_create_with_props(
    struct tinyui_window *parent,
    const struct tinyui_combo_box_props *props);

int tinyui_combo_box_add_item(struct tinyui_combo_box *combo_box, const char *id, const char *text);

int tinyui_combo_box_set_select_item(struct tinyui_combo_box *combo_box, int index);

int tinyui_combo_box_set_selected_index(struct tinyui_combo_box *combo_box, int index);

int tinyui_combo_box_get_select_item(const struct tinyui_combo_box *combo_box);

int tinyui_combo_box_get_selected_index(const struct tinyui_combo_box *combo_box);

const char *tinyui_combo_box_get_text(const struct tinyui_combo_box *combo_box, int index);

int tinyui_combo_box_set_static_items(
    struct tinyui_combo_box *combo_box,
    const char *const *item_ids,
    const char *const *texts,
    int item_count);

int tinyui_combo_box_is_open(const struct tinyui_combo_box *combo_box, int *is_open);

int tinyui_combo_box_set_text_color(struct tinyui_combo_box *combo_box, unsigned int rgb);

int tinyui_combo_box_set_background_color(struct tinyui_combo_box *combo_box, unsigned int rgb);

int tinyui_combo_box_set_bg_color(struct tinyui_combo_box *combo_box, unsigned int rgb);

int tinyui_combo_box_set_frame_color(struct tinyui_combo_box *combo_box, unsigned int rgb);

int tinyui_combo_box_set_select_color(struct tinyui_combo_box *combo_box, unsigned int rgb);

int tinyui_combo_box_set_item_max(struct tinyui_combo_box *combo_box, int item_max);

int tinyui_combo_box_set_dropdown_image(struct tinyui_combo_box *combo_box,
                                        struct tinyui_image_source *source);

int tinyui_combo_box_set_dropdown_source(struct tinyui_combo_box *combo_box,
                                         struct tinyui_image_source *source);

void tinyui_combo_box_set_on_selected(struct tinyui_combo_box *combo_box,
                                      void (*callback)(struct tinyui_combo_box *combo_box,
                                                       int index,
                                                       void *user_data),
                                      void *user_data);

#endif
