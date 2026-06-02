#include "backend.h"

#include <stddef.h>

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
