#include "runtime_state.h"
#include "internal_v1_1.h"

#include "picoui/widget.h"

#include <stddef.h>

static struct picoui_v1_1_widget_binding *picoui_v1_1_widget_binding_find_mutable(
    struct picoui_widget *widget)
{
    struct picoui_runtime_state *state = picoui_runtime_state();
    int i;

    if (widget == 0) {
        return 0;
    }

    for (i = 0; i < PICOUI_V1_1_WIDGET_TREE_CAPACITY; ++i) {
        if (state->widget_tree[i].widget == widget) {
            return &state->widget_tree[i];
        }
    }
    return 0;
}

struct picoui_v1_1_widget_binding *picoui_v1_1_widget_binding_ensure(struct picoui_widget *widget)
{
    struct picoui_runtime_state *state = picoui_runtime_state();
    struct picoui_v1_1_widget_binding *binding;
    int i;

    binding = picoui_v1_1_widget_binding_find_mutable(widget);
    if (binding != 0 || widget == 0) {
        return binding;
    }

    for (i = 0; i < PICOUI_V1_1_WIDGET_TREE_CAPACITY; ++i) {
        if (state->widget_tree[i].widget == 0) {
            state->widget_tree[i].widget = widget;
            return &state->widget_tree[i];
        }
    }

    return 0;
}

const struct picoui_v1_1_widget_binding *picoui_v1_1_widget_binding_find(
    const struct picoui_widget *widget)
{
    return picoui_v1_1_widget_binding_find_mutable((struct picoui_widget *)widget);
}

int picoui_v1_1_widget_bind_root(struct picoui_screen *screen, struct picoui_widget *root)
{
    struct picoui_v1_1_widget_binding *binding;

    if (screen == 0 || root == 0) {
        return -1;
    }

    binding = picoui_v1_1_widget_binding_ensure(root);
    if (binding == 0) {
        return -1;
    }

    binding->screen = screen;
    binding->parent = 0;
    binding->first_child = 0;
    binding->next_sibling = 0;
    binding->root = root;
    return 0;
}

void picoui_v1_1_widget_unbind_root(struct picoui_widget *root)
{
    struct picoui_v1_1_widget_binding *binding;

    binding = picoui_v1_1_widget_binding_find_mutable(root);
    if (binding == 0) {
        return;
    }

    binding->widget = 0;
    binding->screen = 0;
    binding->parent = 0;
    binding->first_child = 0;
    binding->next_sibling = 0;
    binding->root = 0;
}

int picoui_v1_1_widget_append_child(struct picoui_widget *parent, struct picoui_widget *child)
{
    const struct picoui_v1_1_widget_binding *parent_binding;
    struct picoui_v1_1_widget_binding *child_binding;
    struct picoui_v1_1_widget_binding *tail_binding;
    struct picoui_widget *tail;

    if (parent == 0 || child == 0) {
        return -1;
    }

    parent_binding = picoui_v1_1_widget_binding_find(parent);
    if (parent_binding == 0) {
        return -1;
    }

    child_binding = picoui_v1_1_widget_binding_ensure(child);
    if (child_binding == 0) {
        return -1;
    }

    if (child_binding->parent != 0 || child_binding->root != 0 || child_binding->next_sibling != 0
        || child_binding->first_child != 0) {
        return -1;
    }

    child_binding->screen = parent_binding->screen;
    child_binding->parent = parent;
    child_binding->first_child = 0;
    child_binding->next_sibling = 0;
    child_binding->root = parent_binding->root != 0 ? parent_binding->root : parent;

    if (parent_binding->first_child == 0) {
        ((struct picoui_v1_1_widget_binding *)parent_binding)->first_child = child;
        return 0;
    }

    tail = parent_binding->first_child;
    while (tail != 0) {
        tail_binding = picoui_v1_1_widget_binding_ensure(tail);
        if (tail_binding == 0) {
            return -1;
        }
        if (tail_binding->next_sibling == 0) {
            tail_binding->next_sibling = child;
            return 0;
        }
        tail = tail_binding->next_sibling;
    }

    return -1;
}

struct picoui_widget *picoui_widget_get_parent(const struct picoui_widget *widget)
{
    const struct picoui_v1_1_widget_binding *binding = picoui_v1_1_widget_binding_find(widget);

    return binding != 0 ? binding->parent : 0;
}

struct picoui_widget *picoui_widget_get_first_child(const struct picoui_widget *widget)
{
    const struct picoui_v1_1_widget_binding *binding = picoui_v1_1_widget_binding_find(widget);

    return binding != 0 ? binding->first_child : 0;
}

struct picoui_widget *picoui_widget_get_next_sibling(const struct picoui_widget *widget)
{
    const struct picoui_v1_1_widget_binding *binding = picoui_v1_1_widget_binding_find(widget);

    return binding != 0 ? binding->next_sibling : 0;
}

struct picoui_widget *picoui_widget_get_root(const struct picoui_widget *widget)
{
    const struct picoui_v1_1_widget_binding *binding = picoui_v1_1_widget_binding_find(widget);

    return binding != 0 ? binding->root : 0;
}

int picoui_widget_set_text(struct picoui_widget *widget, const char *text)
{
    if (widget == 0 || text == 0) {
        return -1;
    }

    widget->text = text;
    widget->dirty = 1;
    return 0;
}

int picoui_widget_get_child_count(const struct picoui_widget *widget)
{
    struct picoui_widget *child;
    int count = 0;

    if (widget == 0) {
        return -1;
    }

    child = picoui_widget_get_first_child(widget);
    while (child != 0) {
        ++count;
        child = picoui_widget_get_next_sibling(child);
    }

    return count;
}
