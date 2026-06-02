#include "backend.h"
#include "internal.h"
#include "ldList.h"

#include <stdlib.h>

static ldColor picoui_backend_list_rgb_to_ld_color(unsigned int rgb)
{
    return __RGB((rgb >> 16) & 0xFFU, (rgb >> 8) & 0xFFU, rgb & 0xFFU);
}

static arm_2d_align_t picoui_backend_list_map_align(enum picoui_align align)
{
    switch (align) {
    case PICOUI_ALIGN_START:
        return ARM_2D_ALIGN_LEFT;
    case PICOUI_ALIGN_END:
        return ARM_2D_ALIGN_RIGHT;
    case PICOUI_ALIGN_CENTER:
        return ARM_2D_ALIGN_CENTRE;
    default:
        return ARM_2D_ALIGN_CENTRE;
    }
}

static int picoui_backend_list_detach_child(struct picoui_backend_widget *parent,
                                            struct picoui_backend_widget *child)
{
    struct picoui_backend_widget *current;

    if (parent == NULL || child == NULL) {
        return -1;
    }

    if (parent->first_child == child) {
        parent->first_child = child->next_sibling;
        child->next_sibling = NULL;
        child->parent = NULL;
        return 0;
    }

    current = parent->first_child;
    while (current != NULL) {
        if (current->next_sibling == child) {
            current->next_sibling = child->next_sibling;
            child->next_sibling = NULL;
            child->parent = NULL;
            return 0;
        }
        current = current->next_sibling;
    }

    return -1;
}

static int picoui_backend_list_attach_child(struct picoui_backend_widget *parent,
                                            struct picoui_backend_widget *child)
{
    struct picoui_backend_widget *tail;

    if (parent == NULL
        || child == NULL
        || child->kind == PICOUI_BACKEND_WIDGET_WINDOW
        || child->kind == PICOUI_BACKEND_WIDGET_BACKGROUND) {
        return -1;
    }

    child->parent = parent;
    child->root = parent->root;
    child->owner = parent->owner;
    child->next_sibling = NULL;

    if (parent->first_child == NULL) {
        parent->first_child = child;
        return 0;
    }

    tail = parent->first_child;
    while (tail->next_sibling != NULL) {
        tail = tail->next_sibling;
    }
    tail->next_sibling = child;
    return 0;
}

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

int picoui_backend_list_set_item_height(void *backend_widget, int item_height)
{
    struct picoui_backend_widget *widget = backend_widget;

    if (widget == 0 ||
        widget->kind != PICOUI_BACKEND_WIDGET_LIST ||
        widget->ld_widget == 0 ||
        item_height <= 0 ||
        item_height > 255) {
        return -1;
    }

    ldListSetItemHeight((ldList_t *)widget->ld_widget, (uint8_t)item_height);
    return 0;
}

int picoui_backend_list_set_padding_group(void *backend_widget,
                                          int top,
                                          int bottom,
                                          int left,
                                          int right)
{
    struct picoui_backend_widget *widget = backend_widget;

    if (widget == 0 ||
        widget->kind != PICOUI_BACKEND_WIDGET_LIST ||
        widget->ld_widget == 0 ||
        top < 0 || top > 255 ||
        bottom < 0 || bottom > 255 ||
        left < 0 || left > 255 ||
        right < 0 || right > 255) {
        return -1;
    }

    ldListSetPadding((ldList_t *)widget->ld_widget,
                     (uint8_t)top,
                     (uint8_t)bottom,
                     (uint8_t)left,
                     (uint8_t)right);
    return 0;
}

int picoui_backend_list_set_margin_group(void *backend_widget,
                                         int top,
                                         int bottom,
                                         int left,
                                         int right)
{
    struct picoui_backend_widget *widget = backend_widget;

    if (widget == 0 ||
        widget->kind != PICOUI_BACKEND_WIDGET_LIST ||
        widget->ld_widget == 0 ||
        top < 0 || top > 255 ||
        bottom < 0 || bottom > 255 ||
        left < 0 || left > 255 ||
        right < 0 || right > 255) {
        return -1;
    }

    ldListSetMargin((ldList_t *)widget->ld_widget,
                    (uint8_t)top,
                    (uint8_t)bottom,
                    (uint8_t)left,
                    (uint8_t)right);
    return 0;
}

int picoui_backend_list_set_text_color(void *backend_widget, unsigned int rgb)
{
    struct picoui_backend_widget *widget = backend_widget;

    if (widget == 0 ||
        widget->kind != PICOUI_BACKEND_WIDGET_LIST ||
        widget->ld_widget == 0) {
        return -1;
    }

    ldListSetTextColor((ldList_t *)widget->ld_widget, picoui_backend_list_rgb_to_ld_color(rgb));
    return 0;
}

int picoui_backend_list_set_bg_color(void *backend_widget, unsigned int rgb)
{
    struct picoui_backend_widget *widget = backend_widget;

    if (widget == 0 ||
        widget->kind != PICOUI_BACKEND_WIDGET_LIST ||
        widget->ld_widget == 0) {
        return -1;
    }

    ldListSetBackgroundColor((ldList_t *)widget->ld_widget, picoui_backend_list_rgb_to_ld_color(rgb));
    return 0;
}

int picoui_backend_list_set_select_color(void *backend_widget, unsigned int rgb)
{
    struct picoui_backend_widget *widget = backend_widget;

    if (widget == 0 ||
        widget->kind != PICOUI_BACKEND_WIDGET_LIST ||
        widget->ld_widget == 0) {
        return -1;
    }

    ldListSetSelectColor((ldList_t *)widget->ld_widget, picoui_backend_list_rgb_to_ld_color(rgb));
    return 0;
}

int picoui_backend_list_set_align(void *backend_widget, enum picoui_align align)
{
    struct picoui_backend_widget *widget = backend_widget;

    if (widget == 0 ||
        widget->kind != PICOUI_BACKEND_WIDGET_LIST ||
        widget->ld_widget == 0) {
        return -1;
    }

    if (align != PICOUI_ALIGN_START &&
        align != PICOUI_ALIGN_CENTER &&
        align != PICOUI_ALIGN_END) {
        return -1;
    }

    ldListSetAlign((ldList_t *)widget->ld_widget, picoui_backend_list_map_align(align));
    return 0;
}

int picoui_backend_list_set_item_widget(void *backend_widget, int index, void *item_widget_backend)
{
    struct picoui_backend_widget *list_widget = backend_widget;
    struct picoui_backend_widget *item_widget = item_widget_backend;
    ldList_t *ld_list;
    ldBase_t *ld_child;

    if (list_widget == 0 ||
        item_widget == 0 ||
        list_widget->kind != PICOUI_BACKEND_WIDGET_LIST ||
        list_widget->ld_widget == 0 ||
        item_widget->ld_widget == 0 ||
        item_widget->kind == PICOUI_BACKEND_WIDGET_WINDOW ||
        item_widget->kind == PICOUI_BACKEND_WIDGET_BACKGROUND ||
        item_widget->owner != list_widget->owner ||
        index < 0 ||
        index >= list_widget->list_item_count) {
        return -1;
    }

    ld_list = (ldList_t *)list_widget->ld_widget;
    ld_child = (ldBase_t *)item_widget->ld_widget;

    if (item_widget->parent != NULL && item_widget->parent != list_widget) {
        if (picoui_backend_list_detach_child(item_widget->parent, item_widget) != 0) {
            return -1;
        }
        ldBaseNodeRemove((arm_2d_control_node_t *)ld_child);
    } else if (item_widget->parent == list_widget) {
        ldBaseNodeRemove((arm_2d_control_node_t *)ld_child);
        if (picoui_backend_list_detach_child(list_widget, item_widget) != 0) {
            return -1;
        }
    }

    if (picoui_backend_list_attach_child(list_widget, item_widget) != 0) {
        return -1;
    }

    ldBaseNodeAdd((arm_2d_control_node_t *)ld_list, (arm_2d_control_node_t *)ld_child);
    ldListSetItemWidget(ld_list, (uint8_t)index, ld_child);
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

int picoui_backend_list_sync_selected_index(struct picoui_list *list, int *selected_index_out)
{
    struct picoui_backend_widget *backend;
    int selected_index;

    if (list == NULL || list->widget.backend_widget == NULL) {
        return -1;
    }

    backend = (struct picoui_backend_widget *)list->widget.backend_widget;
    selected_index = picoui_backend_list_get_selected_index(backend);
    if (selected_index < -1) {
        return -1;
    }
    if (selected_index >= list->item_count) {
        return -1;
    }

    list->selected_index = selected_index;
    backend->value = selected_index;
    if (selected_index_out != NULL) {
        *selected_index_out = selected_index;
    }
    return 0;
}
