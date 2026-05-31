#include "backend.h"
#include "internal.h"
#include "ldProgressWheel.h"

#include <stdlib.h>

static struct picoui_backend_app_state *picoui_backend_progress_wheel_get_app_state(void *parent)
{
    struct picoui_backend_widget *parent_widget = parent;

    if (parent_widget == NULL || parent_widget->owner == NULL || parent_widget->owner->backend_app == NULL) {
        return NULL;
    }
    return (struct picoui_backend_app_state *)parent_widget->owner->backend_app;
}

static ldProgressWheel_t *picoui_backend_progress_wheel_get_ld(struct picoui_progress_wheel *wheel)
{
    struct picoui_backend_widget *backend;

    if (wheel == NULL || wheel->widget.backend_widget == NULL) {
        return NULL;
    }

    backend = (struct picoui_backend_widget *)wheel->widget.backend_widget;
    if (backend->kind != PICOUI_BACKEND_WIDGET_PROGRESS_WHEEL || backend->ld_widget == NULL) {
        return NULL;
    }

    return (ldProgressWheel_t *)backend->ld_widget;
}

void *picoui_backend_create_progress_wheel(void *parent, const char *id)
{
    struct picoui_backend_widget *widget;
    struct picoui_backend_widget *parent_widget = parent;
    struct picoui_backend_app_state *app_state;
    ldProgressWheel_t *ld_progress_wheel;
    uint16_t name_id;

    if (parent == 0 || id == 0) {
        return 0;
    }

    app_state = picoui_backend_progress_wheel_get_app_state(parent);
    if (app_state == NULL || app_state->ld_scene == NULL || parent_widget->ld_widget == NULL) {
        return 0;
    }

    widget = calloc(1, sizeof(*widget));
    if (widget == 0) {
        return 0;
    }

    name_id = ++app_state->next_ld_name_id;
    ld_progress_wheel = ldProgressWheel_init(app_state->ld_scene,
                                             NULL,
                                             name_id,
                                             parent_widget->ld_name_id,
                                             0,
                                             0,
                                             96,
                                             96);
    if (ld_progress_wheel == NULL) {
        free(widget);
        return 0;
    }

    /* Host PicoUI scenes do not initialize the Arm-2D transform dirty-region helper path. */
    ld_progress_wheel->tWheel.tCFG.bUseDirtyRegions = false;
    ldProgressWheelSetWheelColor(ld_progress_wheel, __RGB(32, 87, 196));
    ldProgressWheelSetDotColor(ld_progress_wheel, GLCD_COLOR_WHITE, true);

    widget->parent = parent;
    widget->id = id;
    widget->kind = PICOUI_BACKEND_WIDGET_PROGRESS_WHEEL;
    widget->theme = ((struct picoui_backend_widget *)parent)->theme;
    widget->ld_widget = ld_progress_wheel;
    widget->ld_name_id = name_id;
    widget->value = 0;
    widget->last_signal = PICOUI_BACKEND_SIGNAL_NONE;
    if (picoui_backend_widget_attach_child(parent, widget) != 0) {
        free(widget);
        return 0;
    }
    return widget;
}

int picoui_backend_progress_wheel_set_percent(struct picoui_progress_wheel *wheel, int percent)
{
    ldProgressWheel_t *ld_progress_wheel = picoui_backend_progress_wheel_get_ld(wheel);
    struct picoui_backend_widget *backend;

    if (ld_progress_wheel == NULL || percent < 0 || percent > 100) {
        return -1;
    }

    ldProgressWheelSetProgress(ld_progress_wheel, (int16_t)(percent * 10));
    backend = (struct picoui_backend_widget *)wheel->widget.backend_widget;
    backend->value = percent;
    return 0;
}

int picoui_backend_progress_wheel_get_percent(struct picoui_progress_wheel *wheel, int *percent)
{
    ldProgressWheel_t *ld_progress_wheel = picoui_backend_progress_wheel_get_ld(wheel);

    if (ld_progress_wheel == NULL || percent == NULL) {
        return -1;
    }

    *percent = ld_progress_wheel->iProgress / 10;
    return 0;
}
