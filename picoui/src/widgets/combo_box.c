#include "internal.h"
#include "picoui/combo_box.h"

#include <stdlib.h>

static int picoui_combo_box_props_are_valid(const struct picoui_combo_box_props *props)
{
    return props != 0
        && props->id != 0
        && props->width >= 0
        && props->height >= 0
        && props->radius >= 0
        && props->padding >= 0;
}

struct picoui_combo_box *picoui_combo_box_create(struct picoui_window *parent, const char *id)
{
    struct picoui_combo_box *combo_box;

    if (parent == 0 || id == 0) {
        return 0;
    }

    combo_box = calloc(1, sizeof(*combo_box));
    if (combo_box == 0) {
        return 0;
    }

    combo_box->widget.backend_widget =
        picoui_backend_create_combo_box(parent->widget.backend_widget, id);
    if (combo_box->widget.backend_widget == 0) {
        free(combo_box);
        return 0;
    }

    combo_box->id = id;
    combo_box->selected_index = -1;
    combo_box->widget.visible = 1;
    combo_box->widget.enabled = 1;
    if (picoui_backend_widget_bind_host(combo_box->widget.backend_widget, &combo_box->widget) != 0) {
        free(combo_box);
        return 0;
    }
    if (picoui_backend_combo_box_bind_host(combo_box->widget.backend_widget) != 0) {
        free(combo_box);
        return 0;
    }
    return combo_box;
}

struct picoui_combo_box *picoui_combo_box_create_with_props(struct picoui_window *parent,
                                                            const struct picoui_combo_box_props *props)
{
    struct picoui_combo_box *combo_box;

    if (!picoui_combo_box_props_are_valid(props)) {
        return 0;
    }

    combo_box = picoui_combo_box_create(parent, props->id);
    if (combo_box == 0) {
        return 0;
    }

    if (picoui_widget_set_user_data(&combo_box->widget, props->user_data) != 0
        || picoui_widget_set_bg_color(&combo_box->widget, props->bg_color) != 0
        || picoui_widget_set_text_color(&combo_box->widget, props->text_color) != 0
        || picoui_widget_set_border_color(&combo_box->widget, props->border_color) != 0
        || picoui_widget_set_radius(&combo_box->widget, props->radius) != 0
        || picoui_widget_set_padding(&combo_box->widget, props->padding) != 0) {
        free(combo_box);
        return 0;
    }
    if (props->style_class != 0
        && picoui_widget_set_style_class(&combo_box->widget, props->style_class) != 0) {
        free(combo_box);
        return 0;
    }
    if ((props->width > 0 || props->height > 0)
        && picoui_widget_set_size(&combo_box->widget, props->width, props->height) != 0) {
        free(combo_box);
        return 0;
    }

    return combo_box;
}

int picoui_combo_box_add_item(struct picoui_combo_box *combo_box, const char *id, const char *text)
{
    int index;
    int next_count;

    if (combo_box == 0 || id == 0 || text == 0 || combo_box->item_count >= PICOUI_LIST_MAX_ITEMS) {
        return -1;
    }

    index = combo_box->item_count;
    combo_box->backend_item_ids[index] = id;
    combo_box->backend_item_texts[index] = (const unsigned char *)text;
    next_count = index + 1;
    if (picoui_backend_combo_box_set_items(combo_box->widget.backend_widget,
                                           combo_box->backend_item_ids,
                                           combo_box->backend_item_texts,
                                           next_count) != 0) {
        combo_box->backend_item_ids[index] = 0;
        combo_box->backend_item_texts[index] = 0;
        return -1;
    }

    combo_box->items[index].id = id;
    combo_box->items[index].text = text;
    combo_box->item_count = next_count;
    return 0;
}

int picoui_combo_box_set_selected_index(struct picoui_combo_box *combo_box, int index)
{
    if (combo_box == 0 || index < 0 || index >= combo_box->item_count) {
        return -1;
    }

    if (picoui_backend_combo_box_set_selected_index(combo_box->widget.backend_widget, index) != 0) {
        return -1;
    }
    combo_box->selected_index = index;
    return 0;
}

int picoui_combo_box_get_selected_index(const struct picoui_combo_box *combo_box)
{
    int backend_selected_index;

    if (combo_box == 0) {
        return -1;
    }

    if (picoui_backend_combo_box_sync_selected_index((struct picoui_combo_box *)combo_box,
                                                     &backend_selected_index) == 0) {
        return backend_selected_index;
    }

    return combo_box->selected_index;
}

int picoui_combo_box_is_open(const struct picoui_combo_box *combo_box, int *is_open)
{
    if (combo_box == 0 || is_open == 0) {
        return -1;
    }

    return picoui_backend_combo_box_get_open((void *)combo_box->widget.backend_widget, is_open);
}

void picoui_combo_box_set_on_selected(struct picoui_combo_box *combo_box,
                                      void (*callback)(struct picoui_combo_box *combo_box,
                                                       int index,
                                                       void *user_data),
                                      void *user_data)
{
    if (combo_box == 0) {
        return;
    }

    combo_box->cb = callback;
    combo_box->user_data = user_data;
}
