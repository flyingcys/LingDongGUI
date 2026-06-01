#include "internal.h"
#include "picoui/icon_slider.h"
#include "picoui/widget.h"

#include <stdlib.h>

static int picoui_icon_slider_props_are_valid(const struct picoui_icon_slider_props *props)
{
    return props != 0 &&
           props->id != 0 &&
           props->width >= 0 &&
           props->height >= 0 &&
           props->icon_width >= 0 &&
           props->icon_space >= 0 &&
           props->columns >= 0 &&
           props->rows >= 0 &&
           props->pages >= 0;
}

struct picoui_icon_slider *picoui_icon_slider_create(struct picoui_widget *parent, const char *id)
{
    struct picoui_icon_slider *icon_slider;

    if (parent == 0 || id == 0 || parent->backend_widget == 0) {
        return 0;
    }

    icon_slider = calloc(1, sizeof(*icon_slider));
    if (icon_slider == 0) {
        return 0;
    }

    icon_slider->widget.backend_widget = picoui_backend_create_icon_slider(parent->backend_widget, id);
    if (icon_slider->widget.backend_widget == 0) {
        free(icon_slider);
        return 0;
    }

    icon_slider->id = id;
    icon_slider->selected_index = -1;
    icon_slider->horizontal = 1;
    icon_slider->icon_width = 48;
    icon_slider->icon_space = 4;
    icon_slider->columns = 4;
    icon_slider->rows = 1;
    icon_slider->pages = 2;
    icon_slider->widget.visible = 1;
    icon_slider->widget.enabled = 1;
    if (picoui_backend_widget_bind_host(icon_slider->widget.backend_widget, &icon_slider->widget) != 0 ||
        picoui_backend_icon_slider_bind_host(icon_slider->widget.backend_widget) != 0) {
        free(icon_slider);
        return 0;
    }

    return icon_slider;
}

struct picoui_icon_slider *picoui_icon_slider_create_with_props(
    struct picoui_widget *parent,
    const struct picoui_icon_slider_props *props
)
{
    struct picoui_icon_slider *icon_slider;

    if (!picoui_icon_slider_props_are_valid(props)) {
        return 0;
    }

    icon_slider = picoui_icon_slider_create(parent, props->id);
    if (icon_slider == 0) {
        return 0;
    }

    if (picoui_widget_set_user_data(&icon_slider->widget, props->user_data) != 0 ||
        (props->style_class != 0 &&
         picoui_widget_set_style_class(&icon_slider->widget, props->style_class) != 0) ||
        ((props->width > 0 || props->height > 0) &&
         picoui_widget_set_size(&icon_slider->widget, props->width, props->height) != 0) ||
        picoui_icon_slider_set_horizontal(icon_slider, props->horizontal != 0) != 0) {
        free(icon_slider);
        return 0;
    }

    icon_slider->icon_width = props->icon_width > 0 ? props->icon_width : icon_slider->icon_width;
    icon_slider->icon_space = props->icon_space;
    icon_slider->columns = props->columns > 0 ? props->columns : icon_slider->columns;
    icon_slider->rows = props->rows > 0 ? props->rows : icon_slider->rows;
    icon_slider->pages = props->pages > 0 ? props->pages : icon_slider->pages;
    return icon_slider;
}

int picoui_icon_slider_add_item(struct picoui_icon_slider *icon_slider, const char *id, const char *text)
{
    int index;

    if (icon_slider == 0 || id == 0 || text == 0 || icon_slider->item_count >= PICOUI_LIST_MAX_ITEMS) {
        return -1;
    }

    if (picoui_backend_icon_slider_add_item(icon_slider->widget.backend_widget, id, text) != 0) {
        return -1;
    }

    index = icon_slider->item_count++;
    icon_slider->items[index].id = id;
    icon_slider->items[index].text = text;
    return 0;
}

int picoui_icon_slider_set_selected_index(struct picoui_icon_slider *icon_slider, int index)
{
    if (icon_slider == 0 || index < 0 || index >= icon_slider->item_count) {
        return -1;
    }

    if (picoui_backend_icon_slider_set_selected_index(icon_slider->widget.backend_widget, index) != 0) {
        return -1;
    }

    icon_slider->selected_index = index;
    return 0;
}

int picoui_icon_slider_get_selected_index(const struct picoui_icon_slider *icon_slider)
{
    int selected_index;

    if (icon_slider == 0) {
        return -1;
    }

    selected_index =
        picoui_backend_icon_slider_get_selected_index((void *)icon_slider->widget.backend_widget);
    if (selected_index >= 0 && selected_index < icon_slider->item_count) {
        ((struct picoui_icon_slider *)icon_slider)->selected_index = selected_index;
        return selected_index;
    }

    return icon_slider->selected_index;
}

int picoui_icon_slider_set_horizontal(struct picoui_icon_slider *icon_slider, int horizontal)
{
    if (icon_slider == 0) {
        return -1;
    }

    if (picoui_backend_icon_slider_set_horizontal(icon_slider->widget.backend_widget, horizontal != 0) != 0) {
        return -1;
    }

    icon_slider->horizontal = horizontal != 0 ? 1 : 0;
    return 0;
}

int picoui_icon_slider_get_horizontal(const struct picoui_icon_slider *icon_slider, int *horizontal)
{
    if (icon_slider == 0 || horizontal == 0) {
        return -1;
    }

    if (picoui_backend_icon_slider_get_horizontal((void *)icon_slider->widget.backend_widget, horizontal) == 0) {
        return 0;
    }

    *horizontal = icon_slider->horizontal;
    return 0;
}

void picoui_icon_slider_set_on_selected(struct picoui_icon_slider *icon_slider,
                                        void (*callback)(struct picoui_icon_slider *icon_slider,
                                                         int index,
                                                         void *user_data),
                                        void *user_data)
{
    if (icon_slider == 0) {
        return;
    }

    icon_slider->cb = callback;
    icon_slider->user_data = user_data;
}
