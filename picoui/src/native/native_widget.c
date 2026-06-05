#include "../backend/ldgui/backend.h"
#include "../core/internal.h"
#include "picoui/screen.h"

#define PICOUI_NATIVE_WIDGET_MAX_BINDINGS 32

struct picoui_native_widget_entry {
    const struct picoui_widget *widget;
    struct picoui_screen *screen;
    const struct picoui_widget *parent;
    const struct picoui_widget *first_child;
    const struct picoui_widget *next_sibling;
    const struct picoui_widget *root;
};

static struct picoui_native_widget_entry g_picoui_native_widget_entries[PICOUI_NATIVE_WIDGET_MAX_BINDINGS];
static int g_picoui_native_widget_entry_count;

struct picoui_widget *picoui_core_widget_get_parent_backend(const struct picoui_widget *widget);
struct picoui_widget *picoui_core_widget_get_first_child_backend(const struct picoui_widget *widget);
struct picoui_widget *picoui_core_widget_get_next_sibling_backend(const struct picoui_widget *widget);
struct picoui_widget *picoui_core_widget_get_root_backend(const struct picoui_widget *widget);
void picoui_native_dirty_mark(const struct picoui_widget *widget, const struct picoui_screen *screen);

static void picoui_native_widget_reset(void)
{
    g_picoui_native_widget_entry_count = 0;
}

static struct picoui_native_widget_entry *picoui_native_widget_find_entry(
    const struct picoui_widget *widget)
{
    int i;

    if (widget == 0) {
        return 0;
    }

    for (i = 0; i < g_picoui_native_widget_entry_count; ++i) {
        if (g_picoui_native_widget_entries[i].widget == widget) {
            return &g_picoui_native_widget_entries[i];
        }
    }

    return 0;
}

static struct picoui_native_widget_entry *picoui_native_widget_alloc_entry(
    const struct picoui_widget *widget)
{
    struct picoui_native_widget_entry *entry;

    entry = picoui_native_widget_find_entry(widget);
    if (entry != 0) {
        return entry;
    }

    if (widget == 0 || g_picoui_native_widget_entry_count >= PICOUI_NATIVE_WIDGET_MAX_BINDINGS) {
        return 0;
    }

    entry = &g_picoui_native_widget_entries[g_picoui_native_widget_entry_count++];
    entry->widget = widget;
    entry->screen = 0;
    entry->parent = 0;
    entry->first_child = 0;
    entry->next_sibling = 0;
    entry->root = 0;
    return entry;
}

static int picoui_native_widget_bind_backend_tree(const struct picoui_backend_widget *backend_widget,
                                                  struct picoui_screen *screen,
                                                  const struct picoui_widget *parent,
                                                  const struct picoui_widget *root)
{
    struct picoui_native_widget_entry *entry;
    const struct picoui_backend_widget *child;

    if (backend_widget == 0 || backend_widget->host_widget == 0) {
        return 0;
    }

    entry = picoui_native_widget_alloc_entry(backend_widget->host_widget);
    if (entry == 0) {
        return -1;
    }

    entry->screen = screen;
    entry->parent = parent;
    entry->root = root;
    entry->first_child = backend_widget->first_child != 0 ? backend_widget->first_child->host_widget : 0;
    entry->next_sibling =
        backend_widget->next_sibling != 0 ? backend_widget->next_sibling->host_widget : 0;

    child = backend_widget->first_child;
    while (child != 0) {
        if (picoui_native_widget_bind_backend_tree(child,
                                                   screen,
                                                   backend_widget->host_widget,
                                                   root) != 0) {
            return -1;
        }
        child = child->next_sibling;
    }

    return 0;
}

int picoui_native_widget_bind_child(struct picoui_widget *parent, struct picoui_widget *child)
{
    struct picoui_native_widget_entry *parent_entry;
    struct picoui_native_widget_entry *child_entry;
    const struct picoui_widget *tail;
    struct picoui_native_widget_entry *tail_entry;

    if (parent == 0 || child == 0) {
        return -1;
    }

    parent_entry = picoui_native_widget_alloc_entry(parent);
    child_entry = picoui_native_widget_alloc_entry(child);
    if (parent_entry == 0 || child_entry == 0) {
        return -1;
    }

    child_entry->screen = parent_entry->screen;
    child_entry->parent = parent;
    child_entry->first_child = 0;
    child_entry->next_sibling = 0;
    child_entry->root = parent_entry->root != 0 ? parent_entry->root : parent;

    if (parent_entry->first_child == 0) {
        parent_entry->first_child = child;
        return 0;
    }

    tail = parent_entry->first_child;
    tail_entry = picoui_native_widget_find_entry(tail);
    while (tail_entry != 0 && tail_entry->next_sibling != 0) {
        tail = tail_entry->next_sibling;
        tail_entry = picoui_native_widget_find_entry(tail);
    }

    if (tail_entry == 0) {
        return -1;
    }

    tail_entry->next_sibling = child;
    return 0;
}

int picoui_native_widget_bind_root(struct picoui_screen *screen, struct picoui_window *root_window)
{
    const struct picoui_backend_widget *root_backend;
    struct picoui_native_widget_entry *root_entry;

    if (screen == 0 || root_window == 0 || root_window->widget.backend_widget == 0) {
        return -1;
    }

    root_backend = (const struct picoui_backend_widget *)root_window->widget.backend_widget;
    picoui_native_widget_reset();
    if (picoui_native_widget_bind_backend_tree(root_backend,
                                               screen,
                                               0,
                                               (const struct picoui_widget *)root_window) != 0) {
        return -1;
    }

    root_entry = picoui_native_widget_alloc_entry((const struct picoui_widget *)root_window);
    if (root_entry == 0) {
        return -1;
    }

    root_entry->screen = screen;
    root_entry->parent = 0;
    root_entry->root = (const struct picoui_widget *)root_window;
    return 0;
}

int picoui_native_widget_is_bound(const struct picoui_widget *widget)
{
    return picoui_native_widget_find_entry(widget) != 0;
}

void picoui_native_widget_mark_dirty(const struct picoui_widget *widget)
{
    struct picoui_native_widget_entry *entry;

    entry = picoui_native_widget_find_entry(widget);
    if (entry == 0) {
        return;
    }

    picoui_native_dirty_mark(widget, entry->screen);
}

struct picoui_widget *picoui_native_widget_get_parent(const struct picoui_widget *widget)
{
    struct picoui_native_widget_entry *entry;

    entry = picoui_native_widget_find_entry(widget);
    return entry != 0 ? (struct picoui_widget *)entry->parent : 0;
}

struct picoui_widget *picoui_native_widget_get_first_child(const struct picoui_widget *widget)
{
    struct picoui_native_widget_entry *entry;

    entry = picoui_native_widget_find_entry(widget);
    return entry != 0 ? (struct picoui_widget *)entry->first_child : 0;
}

struct picoui_widget *picoui_native_widget_get_next_sibling(const struct picoui_widget *widget)
{
    struct picoui_native_widget_entry *entry;

    entry = picoui_native_widget_find_entry(widget);
    return entry != 0 ? (struct picoui_widget *)entry->next_sibling : 0;
}

struct picoui_widget *picoui_native_widget_get_root(const struct picoui_widget *widget)
{
    struct picoui_native_widget_entry *entry;

    entry = picoui_native_widget_find_entry(widget);
    return entry != 0 ? (struct picoui_widget *)entry->root : 0;
}

struct picoui_screen *picoui_native_widget_get_screen(const struct picoui_widget *widget)
{
    struct picoui_native_widget_entry *entry;

    entry = picoui_native_widget_find_entry(widget);
    if (entry == 0) {
        return 0;
    }

    return entry->screen;
}

struct picoui_widget *picoui_widget_get_parent(const struct picoui_widget *widget)
{
    struct picoui_widget *native_parent;

    native_parent = picoui_native_widget_get_parent(widget);
    if (picoui_native_widget_is_bound(widget) != 0) {
        return native_parent;
    }

    return picoui_core_widget_get_parent_backend(widget);
}

struct picoui_widget *picoui_widget_get_first_child(const struct picoui_widget *widget)
{
    struct picoui_widget *native_child;

    native_child = picoui_native_widget_get_first_child(widget);
    if (picoui_native_widget_is_bound(widget) != 0) {
        return native_child;
    }

    return picoui_core_widget_get_first_child_backend(widget);
}

struct picoui_widget *picoui_widget_get_next_sibling(const struct picoui_widget *widget)
{
    struct picoui_widget *native_sibling;

    native_sibling = picoui_native_widget_get_next_sibling(widget);
    if (picoui_native_widget_is_bound(widget) != 0) {
        return native_sibling;
    }

    return picoui_core_widget_get_next_sibling_backend(widget);
}

struct picoui_widget *picoui_widget_get_root(const struct picoui_widget *widget)
{
    struct picoui_widget *native_root;

    native_root = picoui_native_widget_get_root(widget);
    if (picoui_native_widget_is_bound(widget) != 0) {
        return native_root;
    }

    return picoui_core_widget_get_root_backend(widget);
}

struct picoui_screen *picoui_widget_get_screen(const struct picoui_widget *widget)
{
    return picoui_native_widget_get_screen(widget);
}
