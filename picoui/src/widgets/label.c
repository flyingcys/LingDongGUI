#include "internal.h"
#include "picoui/label.h"

#include <stdlib.h>

struct picoui_label *picoui_label_create(struct picoui_window *parent, const char *id)
{
    struct picoui_label *label;

    if (parent == 0 || id == 0) {
        return 0;
    }

    label = calloc(1, sizeof(*label));
    if (label == 0) {
        return 0;
    }

    label->widget.backend_widget = picoui_backend_create_label(parent->widget.backend_widget, id);
    if (label->widget.backend_widget == 0) {
        free(label);
        return 0;
    }

    label->id = id;
    label->widget.visible = 1;
    label->widget.enabled = 1;
    return label;
}

int picoui_label_set_text(struct picoui_label *label, const char *text)
{
    if (label == 0 || text == 0) {
        return -1;
    }

    label->text = text;
    return picoui_backend_set_text(label->widget.backend_widget, text);
}
