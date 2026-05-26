#include "backend.h"
#include "internal.h"
#include "ldCheckBox.h"

#include <stdlib.h>

static struct picoui_backend_app_state *picoui_backend_checkbox_get_app_state(void *parent)
{
    struct picoui_backend_widget *parent_widget = parent;

    if (parent_widget == NULL || parent_widget->owner == NULL || parent_widget->owner->backend_app == NULL) {
        return NULL;
    }
    return (struct picoui_backend_app_state *)parent_widget->owner->backend_app;
}

void *picoui_backend_create_checkbox(void *parent, const char *id)
{
    struct picoui_backend_widget *widget;
    struct picoui_backend_widget *parent_widget = parent;
    struct picoui_backend_app_state *app_state;
    ldCheckBox_t *ld_checkbox;
    uint16_t name_id;

    if (parent == 0 || id == 0) {
        return 0;
    }

    app_state = picoui_backend_checkbox_get_app_state(parent);
    if (app_state == NULL || app_state->ld_scene == NULL || parent_widget->ld_widget == NULL) {
        return 0;
    }

    widget = calloc(1, sizeof(*widget));
    if (widget == 0) {
        return 0;
    }

    name_id = ++app_state->next_ld_name_id;
    ld_checkbox = ldCheckBox_init(app_state->ld_scene,
                                  NULL,
                                  name_id,
                                  parent_widget->ld_name_id,
                                  0,
                                  0,
                                  220,
                                  30);
    if (ld_checkbox == NULL) {
        free(widget);
        return 0;
    }
    ldCheckBoxSetColor(ld_checkbox, __RGB(238, 233, 224), __RGB(32, 87, 196));
    ldCheckBoxSetTextColor(ld_checkbox, __RGB(32, 87, 196));

    widget->parent = parent;
    widget->id = id;
    widget->kind = PICOUI_BACKEND_WIDGET_CHECKBOX;
    widget->theme = ((struct picoui_backend_widget *)parent)->theme;
    widget->ld_widget = ld_checkbox;
    widget->ld_name_id = name_id;
    widget->last_signal = PICOUI_BACKEND_SIGNAL_NONE;
    if (picoui_backend_widget_attach_child(parent, widget) != 0) {
        free(widget);
        return 0;
    }
    return widget;
}
