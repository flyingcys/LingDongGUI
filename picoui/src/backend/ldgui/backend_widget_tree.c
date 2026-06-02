#include "backend.h"
#include "ldBase.h"

#include <stddef.h>

void ldBaseNodeRemove(arm_2d_control_node_t *ptNode);

static int picoui_backend_widget_can_parent(const struct picoui_backend_widget *widget)
{
    if (widget == NULL) {
        return 0;
    }

    return widget->kind == PICOUI_BACKEND_WIDGET_WINDOW;
}

int picoui_backend_widget_is_kind(const void *backend_widget,
                                  enum picoui_backend_widget_kind kind)
{
    const struct picoui_backend_widget *widget = backend_widget;

    if (widget == NULL) {
        return 0;
    }

    return widget->kind == kind;
}

struct picoui_app *picoui_backend_widget_get_owner(void *backend_widget)
{
    struct picoui_backend_widget *widget = backend_widget;

    if (widget == NULL) {
        return NULL;
    }

    return widget->owner;
}

struct picoui_backend_widget *picoui_backend_widget_get_root(void *backend_widget)
{
    struct picoui_backend_widget *widget = backend_widget;

    if (widget == NULL) {
        return NULL;
    }

    return widget->root;
}

struct picoui_backend_widget *picoui_backend_widget_get_parent(void *backend_widget)
{
    struct picoui_backend_widget *widget = backend_widget;

    if (widget == NULL) {
        return NULL;
    }

    return widget->parent;
}

struct picoui_backend_widget *picoui_backend_widget_get_first_child(void *backend_widget)
{
    struct picoui_backend_widget *widget = backend_widget;

    if (widget == NULL) {
        return NULL;
    }

    return widget->first_child;
}

struct picoui_backend_widget *picoui_backend_widget_get_next_sibling(void *backend_widget)
{
    struct picoui_backend_widget *widget = backend_widget;

    if (widget == NULL) {
        return NULL;
    }

    return widget->next_sibling;
}

struct picoui_backend_widget *picoui_backend_widget_find_by_name_id(void *backend_widget, uint16_t name_id)
{
    struct picoui_backend_widget *widget = backend_widget;
    struct picoui_backend_widget *child;
    struct picoui_backend_widget *found;

    if (widget == NULL) {
        return NULL;
    }

    if (widget->ld_name_id == name_id) {
        return widget;
    }

    child = widget->first_child;
    while (child != NULL) {
        found = picoui_backend_widget_find_by_name_id(child, name_id);
        if (found != NULL) {
            return found;
        }
        child = child->next_sibling;
    }

    return NULL;
}

int picoui_backend_widget_attach_child(void *parent, void *child)
{
    struct picoui_backend_widget *parent_widget = parent;
    struct picoui_backend_widget *child_widget = child;

    if (!picoui_backend_widget_can_parent(parent_widget) ||
        child_widget == NULL ||
        child_widget->root != NULL ||
        child_widget->owner != NULL ||
        (child_widget->parent != NULL && child_widget->parent != parent_widget) ||
        child_widget->kind == PICOUI_BACKEND_WIDGET_WINDOW) {
        return -1;
    }

    child_widget->owner = parent_widget->owner;
    child_widget->root = parent_widget->root;
    child_widget->parent = parent_widget;
    child_widget->next_sibling = NULL;
    if (parent_widget->first_child == NULL) {
        parent_widget->first_child = child_widget;
        return 0;
    }

    {
        struct picoui_backend_widget *tail = parent_widget->first_child;
        while (tail->next_sibling != NULL) {
            tail = tail->next_sibling;
        }
        tail->next_sibling = child_widget;
    }

    return 0;
}

static void picoui_backend_widget_clear_owner_and_root(struct picoui_backend_widget *widget)
{
    struct picoui_backend_widget *child;

    if (widget == NULL) {
        return;
    }

    widget->owner = NULL;
    widget->root = NULL;
    child = widget->first_child;
    while (child != NULL) {
        picoui_backend_widget_clear_owner_and_root(child);
        child = child->next_sibling;
    }
}

int picoui_backend_widget_detach_from_parent(void *backend_widget)
{
    struct picoui_backend_widget *widget = backend_widget;
    struct picoui_backend_widget *parent;
    struct picoui_backend_widget *sibling;

    if (widget == NULL || widget->parent == NULL || widget->kind == PICOUI_BACKEND_WIDGET_WINDOW) {
        return -1;
    }

    parent = widget->parent;
    if (parent->first_child == widget) {
        parent->first_child = widget->next_sibling;
    } else {
        sibling = parent->first_child;
        while (sibling != NULL && sibling->next_sibling != widget) {
            sibling = sibling->next_sibling;
        }
        if (sibling == NULL) {
            return -1;
        }
        sibling->next_sibling = widget->next_sibling;
    }

    if (widget->ld_widget != NULL) {
        ldBaseNodeRemove((arm_2d_control_node_t *)widget->ld_widget);
    }
    widget->parent = NULL;
    widget->next_sibling = NULL;
    picoui_backend_widget_clear_owner_and_root(widget);
    return 0;
}

int picoui_backend_widget_unbind_host(void *backend_widget)
{
    struct picoui_backend_widget *widget = backend_widget;

    if (widget == NULL) {
        return -1;
    }

    if (widget->ld_widget != NULL) {
        ((ldBase_t *)widget->ld_widget)->pInfo = NULL;
    }
    widget->host_widget = NULL;
    widget->ld_event_bridge_scene = NULL;
    widget->ld_event_bridge_sender = NULL;
    widget->ld_event_bridge_next = NULL;
    return 0;
}
