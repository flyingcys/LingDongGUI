#include "backend.h"
#include "internal.h"
#include "ldList.h"

#include <stdlib.h>

static struct picoui_backend_app_state *picoui_backend_list_get_app_state(void *parent)
{
    struct picoui_backend_widget *parent_widget = parent;

    if (parent_widget == NULL || parent_widget->owner == NULL || parent_widget->owner->backend_app == NULL) {
        return NULL;
    }
    return (struct picoui_backend_app_state *)parent_widget->owner->backend_app;
}

void *picoui_backend_create_list(void *parent, const char *id)
{
    struct picoui_backend_widget *widget;
    struct picoui_backend_widget *parent_widget = parent;
    struct picoui_backend_app_state *app_state;
    ldList_t *ld_list;
    uint16_t name_id;

    if (parent == 0 || id == 0) {
        return 0;
    }

    app_state = picoui_backend_list_get_app_state(parent);
    if (app_state == NULL || app_state->ld_scene == NULL || parent_widget->ld_widget == NULL) {
        return 0;
    }

    widget = calloc(1, sizeof(*widget));
    if (widget == 0) {
        return 0;
    }

    name_id = ++app_state->next_ld_name_id;
    ld_list = ldList_init(app_state->ld_scene,
                          NULL,
                          name_id,
                          parent_widget->ld_name_id,
                          0,
                          0,
                          220,
                          96);
    if (ld_list == NULL) {
        free(widget);
        return 0;
    }

    ldListSetSelectItem(ld_list, -1);
    widget->parent = parent;
    widget->id = id;
    widget->kind = PICOUI_BACKEND_WIDGET_LIST;
    widget->theme = ((struct picoui_backend_widget *)parent)->theme;
    widget->ld_widget = ld_list;
    widget->ld_name_id = name_id;
    widget->value = -1;
    widget->last_signal = PICOUI_BACKEND_SIGNAL_NONE;
    if (picoui_backend_widget_attach_child(parent, widget) != 0) {
        free(widget);
        return 0;
    }
    return widget;
}

int picoui_backend_list_set_items(void *backend_widget,
                                  const char *const *item_ids,
                                  const unsigned char *const *items,
                                  int item_count)
{
    struct picoui_backend_widget *widget = backend_widget;
    int i;

    if (widget == 0 ||
        widget->kind != PICOUI_BACKEND_WIDGET_LIST ||
        widget->ld_widget == 0 ||
        item_ids == 0 ||
        items == 0 ||
        item_count < 0 ||
        item_count > PICOUI_BACKEND_LIST_MAX_ITEMS) {
        return -1;
    }

    ldListSetText((ldList_t *)widget->ld_widget,
                  (const uint8_t **)items,
                  (uint8_t)item_count,
                  NULL);
    for (i = 0; i < item_count; ++i) {
        widget->list_item_ids[i] = item_ids[i];
    }
    widget->list_item_count = item_count;
    return 0;
}

int picoui_backend_list_set_selected_index(void *backend_widget, int index)
{
    struct picoui_backend_widget *widget = backend_widget;

    if (widget == 0 ||
        widget->kind != PICOUI_BACKEND_WIDGET_LIST ||
        widget->ld_widget == 0 ||
        index < 0 ||
        index >= PICOUI_BACKEND_LIST_MAX_ITEMS) {
        return -1;
    }

    ldListSetSelectItem((ldList_t *)widget->ld_widget, (int8_t)index);
    widget->value = index;
    return 0;
}

int picoui_backend_list_get_selected_index(void *backend_widget)
{
    struct picoui_backend_widget *widget = backend_widget;

    if (widget == 0 ||
        widget->kind != PICOUI_BACKEND_WIDGET_LIST ||
        widget->ld_widget == 0) {
        return -1;
    }

    return ldListGetSelectItem((ldList_t *)widget->ld_widget);
}
