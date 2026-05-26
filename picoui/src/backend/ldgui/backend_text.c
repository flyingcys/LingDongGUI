#include "backend.h"

#include <stdlib.h>

void *picoui_backend_create_text(void *parent, const char *id)
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
    widget->kind = PICOUI_BACKEND_WIDGET_TEXT;
    widget->theme = ((struct picoui_backend_widget *)parent)->theme;
    if (picoui_backend_widget_attach_child(parent, widget) != 0) {
        free(widget);
        return 0;
    }
    return widget;
}
