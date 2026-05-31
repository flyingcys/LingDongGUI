#include "backend.h"
#include "internal.h"
#include "ldDateTime.h"

#include <stdlib.h>

extern const arm_2d_a1_font_t ARM_2D_FONT_6x8;

static struct picoui_backend_app_state *picoui_backend_date_time_get_app_state(void *parent)
{
    struct picoui_backend_widget *parent_widget = parent;

    if (parent_widget == NULL || parent_widget->owner == NULL || parent_widget->owner->backend_app == NULL) {
        return NULL;
    }
    return (struct picoui_backend_app_state *)parent_widget->owner->backend_app;
}

static ldDateTime_t *picoui_backend_date_time_get_ld(struct picoui_date_time *dt)
{
    struct picoui_backend_widget *backend;

    if (dt == NULL || dt->widget.backend_widget == NULL) {
        return NULL;
    }

    backend = (struct picoui_backend_widget *)dt->widget.backend_widget;
    if (backend->kind != PICOUI_BACKEND_WIDGET_DATE_TIME || backend->ld_widget == NULL) {
        return NULL;
    }

    return (ldDateTime_t *)backend->ld_widget;
}

void *picoui_backend_create_date_time(void *parent, const char *id)
{
    struct picoui_backend_widget *widget;
    struct picoui_backend_widget *parent_widget = parent;
    struct picoui_backend_app_state *app_state;
    ldDateTime_t *ld_date_time;
    uint16_t name_id;

    if (parent == 0 || id == 0) {
        return 0;
    }

    app_state = picoui_backend_date_time_get_app_state(parent);
    if (app_state == NULL || app_state->ld_scene == NULL || parent_widget->ld_widget == NULL) {
        return 0;
    }

    widget = calloc(1, sizeof(*widget));
    if (widget == 0) {
        return 0;
    }

    name_id = ++app_state->next_ld_name_id;
    ld_date_time = ldDateTime_init(app_state->ld_scene,
                                   NULL,
                                   name_id,
                                   parent_widget->ld_name_id,
                                   0,
                                   0,
                                   240,
                                   32,
                                   (arm_2d_font_t *)&ARM_2D_FONT_6x8);
    if (ld_date_time == NULL) {
        free(widget);
        return 0;
    }

    widget->parent = parent;
    widget->id = id;
    widget->kind = PICOUI_BACKEND_WIDGET_DATE_TIME;
    widget->theme = ((struct picoui_backend_widget *)parent)->theme;
    widget->ld_widget = ld_date_time;
    widget->ld_name_id = name_id;
    widget->text = (const char *)ld_date_time->formatStr;
    if (picoui_backend_widget_attach_child(parent, widget) != 0) {
        free(widget);
        return 0;
    }
    return widget;
}

int picoui_backend_date_time_set_format(struct picoui_date_time *dt, const char *format)
{
    ldDateTime_t *ld_date_time = picoui_backend_date_time_get_ld(dt);
    struct picoui_backend_widget *backend;

    if (ld_date_time == NULL || format == NULL) {
        return -1;
    }

    ldDateTimeSetFormat(ld_date_time, (const uint8_t *)format);
    backend = (struct picoui_backend_widget *)dt->widget.backend_widget;
    backend->text = format;
    return 0;
}

int picoui_backend_date_time_set_date(struct picoui_date_time *dt, int year, int month, int day)
{
    ldDateTime_t *ld_date_time = picoui_backend_date_time_get_ld(dt);

    if (ld_date_time == NULL) {
        return -1;
    }

    ldDateTimeSetDate(ld_date_time, (uint16_t)year, (uint8_t)month, (uint8_t)day);
    return 0;
}

int picoui_backend_date_time_set_time(struct picoui_date_time *dt, int hour, int minute, int second)
{
    ldDateTime_t *ld_date_time = picoui_backend_date_time_get_ld(dt);

    if (ld_date_time == NULL) {
        return -1;
    }

    ldDateTimeSetTime(ld_date_time, (uint8_t)hour, (uint8_t)minute, (uint8_t)second);
    return 0;
}

const char *picoui_backend_date_time_get_format(struct picoui_date_time *dt)
{
    ldDateTime_t *ld_date_time = picoui_backend_date_time_get_ld(dt);

    if (ld_date_time == NULL) {
        return NULL;
    }

    return (const char *)ld_date_time->formatStr;
}
