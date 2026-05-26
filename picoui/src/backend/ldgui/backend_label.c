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
    widget->kind = PICOUI_BACKEND_WIDGET_LABEL;
    widget->theme = ((struct picoui_backend_widget *)parent)->theme;
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

int picoui_backend_widget_set_style_class(void *backend_widget, const char *style_class)
{
    struct picoui_backend_widget *widget = backend_widget;

    if (widget == 0) {
        return -1;
    }

    widget->style_class = style_class;
    return 0;
}

int picoui_backend_widget_set_font(void *backend_widget, const void *font)
{
    struct picoui_backend_widget *widget = backend_widget;

    if (widget == 0) {
        return -1;
    }

    widget->font = font;
    return 0;
}

int picoui_backend_widget_set_user_data(void *backend_widget, void *user_data)
{
    struct picoui_backend_widget *widget = backend_widget;

    if (widget == 0) {
        return -1;
    }

    widget->user_data = user_data;
    return 0;
}
