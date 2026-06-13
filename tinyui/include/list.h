#ifndef TINYUI_LIST_H
#define TINYUI_LIST_H

#include "widget.h"

struct tinyui_widget;
struct tinyui_list;

struct tinyui_list_props {
    const char *id;
    const char *style_class;
    void *user_data;
};

struct tinyui_list *tinyui_list_create(struct tinyui_widget *parent, const char *id);

struct tinyui_list *tinyui_list_create_with_props(
    struct tinyui_widget *parent,
    const struct tinyui_list_props *props);

int tinyui_list_add_item(struct tinyui_list *list, const char *id, const char *text);

int tinyui_list_set_item_height(struct tinyui_list *list, int item_height);

int tinyui_list_set_padding_group(struct tinyui_list *list, int top, int bottom, int left, int right);

int tinyui_list_set_margin_group(struct tinyui_list *list, int top, int bottom, int left, int right);

int tinyui_list_set_text_color(struct tinyui_list *list, unsigned int rgb);

int tinyui_list_set_bg_color(struct tinyui_list *list, unsigned int rgb);

int tinyui_list_set_select_color(struct tinyui_list *list, unsigned int rgb);

int tinyui_list_set_align(struct tinyui_list *list, enum tinyui_align align);

int tinyui_list_set_item_widget(struct tinyui_list *list,
                                int index,
                                struct tinyui_widget *item_widget);

int tinyui_list_set_selected_index(struct tinyui_list *list, int index);

int tinyui_list_get_selected_index(const struct tinyui_list *list);

void tinyui_list_set_on_selected(struct tinyui_list *list,
                                 void (*callback)(struct tinyui_list *list,
                                                  int index,
                                                  void *user_data),
                                 void *user_data);

#endif
