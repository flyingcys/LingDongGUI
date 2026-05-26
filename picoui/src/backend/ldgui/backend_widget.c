#include "backend.h"

#include <stddef.h>

/* Shared backend widget helpers live here when ldgui-specific widget state grows. */

int picoui_backend_widget_attach_child(void *parent, void *child)
{
    struct picoui_backend_widget *parent_widget = parent;
    struct picoui_backend_widget *child_widget = child;

    if (parent_widget == NULL || child_widget == NULL) {
        return -1;
    }

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
