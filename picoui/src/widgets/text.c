#include "internal.h"
#include "picoui/text.h"

#include <stdlib.h>

struct picoui_text *picoui_text_create(struct picoui_window *parent, const char *id)
{
    struct picoui_text *text;

    if (parent == 0 || id == 0) {
        return 0;
    }

    text = calloc(1, sizeof(*text));
    if (text == 0) {
        return 0;
    }

    text->widget.backend_widget = picoui_backend_create_text(parent->widget.backend_widget, id);
    if (text->widget.backend_widget == 0) {
        free(text);
        return 0;
    }

    text->id = id;
    text->widget.visible = 1;
    text->widget.enabled = 1;
    return text;
}

int picoui_text_set_text(struct picoui_text *text, const char *value)
{
    if (text == 0 || value == 0) {
        return -1;
    }

    text->text = value;
    return picoui_backend_set_text(text->widget.backend_widget, value);
}
