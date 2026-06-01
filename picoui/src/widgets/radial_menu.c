#include "internal.h"
#include "picoui/radial_menu.h"
#include "picoui/widget.h"

#include <stdlib.h>

static int picoui_radial_menu_props_are_valid(const struct picoui_radial_menu_props *props)
{
    return props != 0 &&
           props->id != 0 &&
           props->width >= 0 &&
           props->height >= 0 &&
           props->x_axis >= 0 &&
           props->y_axis >= 0 &&
           props->item_max >= 0 &&
           props->default_index >= -1;
}

struct picoui_radial_menu *picoui_radial_menu_create(struct picoui_widget *parent, const char *id)
{
    struct picoui_radial_menu *radial_menu;

    if (parent == 0 || id == 0 || parent->backend_widget == 0) {
        return 0;
    }

    radial_menu = calloc(1, sizeof(*radial_menu));
    if (radial_menu == 0) {
        return 0;
    }

    radial_menu->widget.backend_widget = picoui_backend_create_radial_menu(parent->backend_widget, id);
    if (radial_menu->widget.backend_widget == 0) {
        free(radial_menu);
        return 0;
    }

    radial_menu->id = id;
    radial_menu->selected_index = -1;
    radial_menu->x_axis = 68;
    radial_menu->y_axis = 46;
    radial_menu->item_max = 5;
    radial_menu->widget.visible = 1;
    radial_menu->widget.enabled = 1;
    if (picoui_backend_widget_bind_host(radial_menu->widget.backend_widget, &radial_menu->widget) != 0 ||
        picoui_backend_radial_menu_bind_host(radial_menu->widget.backend_widget) != 0) {
        free(radial_menu);
        return 0;
    }

    return radial_menu;
}

struct picoui_radial_menu *picoui_radial_menu_create_with_props(
    struct picoui_widget *parent,
    const struct picoui_radial_menu_props *props
)
{
    struct picoui_radial_menu *radial_menu;

    if (!picoui_radial_menu_props_are_valid(props)) {
        return 0;
    }

    radial_menu = picoui_radial_menu_create(parent, props->id);
    if (radial_menu == 0) {
        return 0;
    }

    if (picoui_widget_set_user_data(&radial_menu->widget, props->user_data) != 0 ||
        (props->style_class != 0 &&
         picoui_widget_set_style_class(&radial_menu->widget, props->style_class) != 0) ||
        ((props->width > 0 || props->height > 0) &&
         picoui_widget_set_size(&radial_menu->widget, props->width, props->height) != 0)) {
        free(radial_menu);
        return 0;
    }

    radial_menu->x_axis = props->x_axis > 0 ? props->x_axis : radial_menu->x_axis;
    radial_menu->y_axis = props->y_axis > 0 ? props->y_axis : radial_menu->y_axis;
    radial_menu->item_max = props->item_max > 0 ? props->item_max : radial_menu->item_max;
    if (props->default_index >= 0) {
        radial_menu->selected_index = props->default_index;
    }
    return radial_menu;
}

int picoui_radial_menu_add_item(struct picoui_radial_menu *radial_menu, const char *id)
{
    int index;

    if (radial_menu == 0 || id == 0 || radial_menu->item_count >= PICOUI_LIST_MAX_ITEMS) {
        return -1;
    }

    if (picoui_backend_radial_menu_add_item(radial_menu->widget.backend_widget, id) != 0) {
        return -1;
    }

    index = radial_menu->item_count++;
    radial_menu->items[index].id = id;
    radial_menu->items[index].text = id;
    if (radial_menu->selected_index < 0) {
        radial_menu->selected_index = 0;
    }
    if (radial_menu->selected_index >= 0 &&
        radial_menu->selected_index < radial_menu->item_count &&
        picoui_backend_radial_menu_set_selected_index(radial_menu->widget.backend_widget,
                                                      radial_menu->selected_index) != 0) {
        radial_menu->item_count--;
        radial_menu->items[index].id = 0;
        radial_menu->items[index].text = 0;
        if (radial_menu->item_count == 0) {
            radial_menu->selected_index = -1;
        }
        return -1;
    }
    return 0;
}

int picoui_radial_menu_set_selected_index(struct picoui_radial_menu *radial_menu, int index)
{
    if (radial_menu == 0 || index < 0 || index >= radial_menu->item_count) {
        return -1;
    }

    if (picoui_backend_radial_menu_set_selected_index(radial_menu->widget.backend_widget, index) != 0) {
        return -1;
    }

    radial_menu->selected_index = index;
    return 0;
}

int picoui_radial_menu_get_selected_index(const struct picoui_radial_menu *radial_menu)
{
    int selected_index;

    if (radial_menu == 0) {
        return -1;
    }

    selected_index =
        picoui_backend_radial_menu_get_selected_index((void *)radial_menu->widget.backend_widget);
    if (selected_index >= 0 && selected_index < radial_menu->item_count) {
        ((struct picoui_radial_menu *)radial_menu)->selected_index = selected_index;
        return selected_index;
    }

    return radial_menu->selected_index;
}

int picoui_radial_menu_offset_selection(struct picoui_radial_menu *radial_menu, int offset)
{
    int selected_index;

    if (radial_menu == 0 || radial_menu->item_count <= 0) {
        return -1;
    }

    if (picoui_backend_radial_menu_offset_selection(radial_menu->widget.backend_widget, offset) != 0) {
        return -1;
    }

    selected_index = picoui_backend_radial_menu_get_selected_index(radial_menu->widget.backend_widget);
    if (selected_index >= 0 && selected_index < radial_menu->item_count) {
        radial_menu->selected_index = selected_index;
    }
    return 0;
}

void picoui_radial_menu_set_on_selected(struct picoui_radial_menu *radial_menu,
                                        void (*callback)(struct picoui_radial_menu *radial_menu,
                                                         int index,
                                                         void *user_data),
                                        void *user_data)
{
    if (radial_menu == 0) {
        return;
    }

    radial_menu->cb = callback;
    radial_menu->user_data = user_data;
}
