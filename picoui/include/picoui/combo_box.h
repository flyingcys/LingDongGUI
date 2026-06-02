#ifndef PICOUI_COMBO_BOX_H
#define PICOUI_COMBO_BOX_H

#include "picoui/widget.h"

struct picoui_window;
struct picoui_combo_box;
struct picoui_image_source;

struct picoui_combo_box_props {
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

struct picoui_combo_box *picoui_combo_box_create(struct picoui_window *parent, const char *id);
struct picoui_combo_box *picoui_combo_box_create_with_props(struct picoui_window *parent,
                                                            const struct picoui_combo_box_props *props);
int picoui_combo_box_add_item(struct picoui_combo_box *combo_box, const char *id, const char *text);
int picoui_combo_box_set_select_item(struct picoui_combo_box *combo_box, int index);
int picoui_combo_box_set_selected_index(struct picoui_combo_box *combo_box, int index);
int picoui_combo_box_get_select_item(const struct picoui_combo_box *combo_box);
int picoui_combo_box_get_selected_index(const struct picoui_combo_box *combo_box);
const char *picoui_combo_box_get_text(const struct picoui_combo_box *combo_box, int index);
int picoui_combo_box_set_static_items(struct picoui_combo_box *combo_box,
                                      const char *const *item_ids,
                                      const char *const *texts,
                                      int item_count);
int picoui_combo_box_is_open(const struct picoui_combo_box *combo_box, int *is_open);
int picoui_combo_box_set_text_color(struct picoui_combo_box *combo_box, unsigned int rgb);
int picoui_combo_box_set_background_color(struct picoui_combo_box *combo_box, unsigned int rgb);
int picoui_combo_box_set_bg_color(struct picoui_combo_box *combo_box, unsigned int rgb);
int picoui_combo_box_set_frame_color(struct picoui_combo_box *combo_box, unsigned int rgb);
int picoui_combo_box_set_select_color(struct picoui_combo_box *combo_box, unsigned int rgb);
int picoui_combo_box_set_item_max(struct picoui_combo_box *combo_box, int item_max);
int picoui_combo_box_set_dropdown_image(struct picoui_combo_box *combo_box,
                                        struct picoui_image_source *source);
int picoui_combo_box_set_dropdown_source(struct picoui_combo_box *combo_box,
                                         struct picoui_image_source *source);
void picoui_combo_box_set_on_selected(struct picoui_combo_box *combo_box,
                                      void (*callback)(struct picoui_combo_box *combo_box,
                                                       int index,
                                                       void *user_data),
                                      void *user_data);

#endif
