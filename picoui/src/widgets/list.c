#include "internal.h"
#include "picoui/list.h"
#include "picoui/widget.h"

#include <stdlib.h>

struct picoui_list *picoui_list_create(struct picoui_widget *parent, const char *id)
{
    struct picoui_list *list;

    if (parent == 0 || id == 0 || parent->backend_widget == 0) {
        return 0;
    }

    list = calloc(1, sizeof(*list));
    if (list == 0) {
        return 0;
    }

    list->widget.backend_widget = picoui_backend_create_list(parent->backend_widget, id);
    if (list->widget.backend_widget == 0) {
        free(list);
        return 0;
    }

    list->id = id;
    list->selected_index = -1;
    list->widget.visible = 1;
    list->widget.enabled = 1;
    if (picoui_backend_widget_bind_host(list->widget.backend_widget, &list->widget) != 0) {
        free(list);
        return 0;
    }
    return list;
}

struct picoui_list *picoui_list_create_with_props(struct picoui_widget *parent,
                                                  const struct picoui_list_props *props)
{
    struct picoui_list *list;

    if (props == 0) {
        return 0;
    }

    list = picoui_list_create(parent, props->id);
    if (list == 0) {
        return 0;
    }

    list->user_data = props->user_data;
    if (picoui_widget_set_style_class(&list->widget, props->style_class) != 0 ||
        picoui_widget_set_user_data(&list->widget, props->user_data) != 0) {
        free(list);
        return 0;
    }
    return list;
}

int picoui_list_add_item(struct picoui_list *list, const char *id, const char *text)
{
    int index;
    int next_count;

    if (list == 0 || id == 0 || text == 0 || list->item_count >= PICOUI_LIST_MAX_ITEMS) {
        return -1;
    }

    index = list->item_count;
    list->backend_item_ids[index] = id;
    list->backend_item_texts[index] = (const unsigned char *)text;
    next_count = index + 1;

    if (picoui_backend_list_set_items(list->widget.backend_widget,
                                      list->backend_item_ids,
                                      list->backend_item_texts,
                                      next_count) != 0) {
        list->backend_item_ids[index] = 0;
        list->backend_item_texts[index] = 0;
        return -1;
    }

    list->items[index].id = id;
    list->items[index].text = text;
    list->item_count = next_count;
    return 0;
}

int picoui_list_set_item_height(struct picoui_list *list, int item_height)
{
    if (list == 0) {
        return -1;
    }

    return picoui_backend_list_set_item_height(list->widget.backend_widget, item_height);
}

int picoui_list_set_padding_group(struct picoui_list *list, int top, int bottom, int left, int right)
{
    if (list == 0) {
        return -1;
    }

    return picoui_backend_list_set_padding_group(list->widget.backend_widget, top, bottom, left, right);
}

int picoui_list_set_margin_group(struct picoui_list *list, int top, int bottom, int left, int right)
{
    if (list == 0) {
        return -1;
    }

    return picoui_backend_list_set_margin_group(list->widget.backend_widget, top, bottom, left, right);
}

int picoui_list_set_text_color(struct picoui_list *list, unsigned int rgb)
{
    if (list == 0 || picoui_widget_set_text_color(&list->widget, rgb) != 0) {
        return -1;
    }

    return picoui_backend_list_set_text_color(list->widget.backend_widget, rgb);
}

int picoui_list_set_bg_color(struct picoui_list *list, unsigned int rgb)
{
    if (list == 0 || picoui_widget_set_bg_color(&list->widget, rgb) != 0) {
        return -1;
    }

    return picoui_backend_list_set_bg_color(list->widget.backend_widget, rgb);
}

int picoui_list_set_select_color(struct picoui_list *list, unsigned int rgb)
{
    if (list == 0 || picoui_widget_set_border_color(&list->widget, rgb) != 0) {
        return -1;
    }

    return picoui_backend_list_set_select_color(list->widget.backend_widget, rgb);
}

int picoui_list_set_align(struct picoui_list *list, enum picoui_align align)
{
    if (list == 0) {
        return -1;
    }

    return picoui_backend_list_set_align(list->widget.backend_widget, align);
}

int picoui_list_set_item_widget(struct picoui_list *list,
                                int index,
                                struct picoui_widget *item_widget)
{
    if (list == 0 || item_widget == 0 || item_widget->backend_widget == 0) {
        return -1;
    }

    return picoui_backend_list_set_item_widget(list->widget.backend_widget,
                                               index,
                                               item_widget->backend_widget);
}

int picoui_list_set_selected_index(struct picoui_list *list, int index)
{
    if (list == 0 || index < 0 || index >= list->item_count) {
        return -1;
    }

    if (picoui_backend_list_set_selected_index(list->widget.backend_widget, index) != 0) {
        return -1;
    }
    list->selected_index = index;
    return 0;
}

int picoui_list_get_selected_index(const struct picoui_list *list)
{
    int backend_selected_index;

    if (list == 0) {
        return -1;
    }

    if (picoui_backend_list_sync_selected_index((struct picoui_list *)list, &backend_selected_index) == 0) {
        return backend_selected_index;
    }

    return list->selected_index;
}

void picoui_list_set_on_selected(struct picoui_list *list,
                                 void (*callback)(struct picoui_list *list,
                                                  int index,
                                                  void *user_data),
                                 void *user_data)
{
    if (list == 0) {
        return;
    }

    list->cb = callback;
    list->user_data = user_data;
}
