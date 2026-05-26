#include "backend.h"

#include <stdlib.h>

void *picoui_backend_create_checkbox(void *parent, const char *id)
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
    widget->kind = PICOUI_BACKEND_WIDGET_CHECKBOX;
    widget->theme = ((struct picoui_backend_widget *)parent)->theme;
    widget->last_signal = PICOUI_BACKEND_SIGNAL_NONE;
    return widget;
}
