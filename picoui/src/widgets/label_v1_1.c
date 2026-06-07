#include "runtime_state.h"
#include "internal_v1_1.h"

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

    label->id = id;
    label->widget.visible = 1;
    label->widget.enabled = 1;

    if (picoui_v1_1_widget_append_child(&parent->widget, &label->widget) != 0) {
        free(label);
        return 0;
    }

    return label;
}

int picoui_label_set_text(struct picoui_label *label, const char *text)
{
    if (label == 0 || text == 0) {
        return -1;
    }

    label->widget.text = text;
    label->widget.dirty = 1;
    return 0;
}

const char *picoui_label_get_text(struct picoui_label *label)
{
    if (label == 0) {
        return 0;
    }

    return label->widget.text;
}
