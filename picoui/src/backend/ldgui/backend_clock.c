#include "backend.h"
#include "internal.h"
#include "ldClock.h"

#include <stdlib.h>

static struct picoui_backend_app_state *picoui_backend_clock_get_app_state(void *parent)
{
    struct picoui_backend_widget *parent_widget = parent;

    if (parent_widget == NULL || parent_widget->owner == NULL || parent_widget->owner->backend_app == NULL) {
        return NULL;
    }
    return (struct picoui_backend_app_state *)parent_widget->owner->backend_app;
}

static ldClock_t *picoui_backend_clock_get_ld(struct picoui_clock *clock)
{
    struct picoui_backend_widget *backend;

    if (clock == NULL || clock->widget.backend_widget == NULL) {
        return NULL;
    }

    backend = (struct picoui_backend_widget *)clock->widget.backend_widget;
    if (backend->kind != PICOUI_BACKEND_WIDGET_CLOCK || backend->ld_widget == NULL) {
        return NULL;
    }

    return (ldClock_t *)backend->ld_widget;
}

void *picoui_backend_create_clock(void *parent, const char *id)
{
    struct picoui_backend_widget *widget;
    struct picoui_backend_widget *parent_widget = parent;
    struct picoui_backend_app_state *app_state;
    ldClock_t *ld_clock;
    uint16_t name_id;

    if (parent == 0 || id == 0) {
        return 0;
    }

    app_state = picoui_backend_clock_get_app_state(parent);
    if (app_state == NULL || app_state->ld_scene == NULL || parent_widget->ld_widget == NULL) {
        return 0;
    }

    widget = calloc(1, sizeof(*widget));
    if (widget == 0) {
        return 0;
    }

    name_id = ++app_state->next_ld_name_id;
    ld_clock = ldClock_init(app_state->ld_scene,
                            NULL,
                            name_id,
                            parent_widget->ld_name_id,
                            0,
                            0,
                            200,
                            200);
    if (ld_clock == NULL) {
        free(widget);
        return 0;
    }

    widget->parent = parent;
    widget->id = id;
    widget->kind = PICOUI_BACKEND_WIDGET_CLOCK;
    widget->theme = ((struct picoui_backend_widget *)parent)->theme;
    widget->ld_widget = ld_clock;
    widget->ld_name_id = name_id;
    widget->value = 0;
    if (picoui_backend_widget_attach_child(parent, widget) != 0) {
        free(widget);
        return 0;
    }
    return widget;
}

int picoui_backend_clock_set_step_second(struct picoui_clock *clock, int step_second)
{
    ldClock_t *ld_clock = picoui_backend_clock_get_ld(clock);
    struct picoui_backend_widget *backend;

    if (ld_clock == NULL || (step_second != 0 && step_second != 1)) {
        return -1;
    }

    ldClockSetStepSecond(ld_clock, step_second != 0);
    backend = (struct picoui_backend_widget *)clock->widget.backend_widget;
    backend->value = step_second;
    return 0;
}

int picoui_backend_clock_get_step_second(struct picoui_clock *clock, int *step_second)
{
    ldClock_t *ld_clock = picoui_backend_clock_get_ld(clock);

    if (ld_clock == NULL || step_second == NULL) {
        return -1;
    }

    *step_second = ld_clock->isStepSecond ? 1 : 0;
    return 0;
}
