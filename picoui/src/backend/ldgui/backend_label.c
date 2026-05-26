#include "backend.h"

#include <stdlib.h>

void *picoui_backend_create_label(void *parent, const char *id)
{
    struct picoui_backend_widget *widget;

    if (parent == 0 || id == 0) {
        return 0;
    }

    widget = calloc(1, sizeof(*widget));
    if (widget == 0) {
        return 0;
    }

    widget->parent = parent;
    widget->id = id;
    return widget;
}

int picoui_backend_set_text(void *backend_widget, const char *text)
{
    struct picoui_backend_widget *widget = backend_widget;

    if (widget == 0 || text == 0) {
        return -1;
    }

    widget->text = text;
    return 0;
}
