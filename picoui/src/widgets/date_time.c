#include "internal.h"
#include "picoui/date_time.h"
#include "picoui/widget.h"

#include <stdlib.h>

int picoui_backend_date_time_set_format(struct picoui_date_time *dt, const char *format);
int picoui_backend_date_time_set_date(struct picoui_date_time *dt, int year, int month, int day);
int picoui_backend_date_time_set_time(struct picoui_date_time *dt, int hour, int minute, int second);
const char *picoui_backend_date_time_get_format(struct picoui_date_time *dt);

static int picoui_date_time_props_are_valid(const struct picoui_date_time_props *props)
{
    return props != 0
        && props->id != 0
        && props->format != 0
        && props->month >= 1
        && props->month <= 12
        && props->day >= 1
        && props->day <= 31
        && props->hour >= 0
        && props->hour <= 23
        && props->minute >= 0
        && props->minute <= 59
        && props->second >= 0
        && props->second <= 59;
}

struct picoui_date_time *picoui_date_time_create(struct picoui_widget *parent, const char *id)
{
    struct picoui_date_time *dt;

    if (parent == 0 || id == 0 || parent->backend_widget == 0) {
        return 0;
    }

    dt = calloc(1, sizeof(*dt));
    if (dt == 0) {
        return 0;
    }

    dt->widget.backend_widget = picoui_backend_create_date_time(parent->backend_widget, id);
    if (dt->widget.backend_widget == 0) {
        free(dt);
        return 0;
    }

    dt->id = id;
    dt->widget.visible = 1;
    dt->widget.enabled = 1;
    if (picoui_backend_widget_bind_host(dt->widget.backend_widget, &dt->widget) != 0) {
        free(dt);
        return 0;
    }
    if (picoui_date_time_set_format(dt, "yyyy-mm-dd hh:nn:ss") != 0
        || picoui_date_time_set_date(dt, 2026, 1, 1) != 0
        || picoui_date_time_set_time(dt, 12, 0, 0) != 0) {
        free(dt);
        return 0;
    }
    return dt;
}

struct picoui_date_time *picoui_date_time_create_with_props(
    struct picoui_widget *parent,
    const struct picoui_date_time_props *props)
{
    struct picoui_date_time *dt;

    if (!picoui_date_time_props_are_valid(props)) {
        return 0;
    }

    dt = picoui_date_time_create(parent, props->id);
    if (dt == 0) {
        return 0;
    }

    if (props->style_class != 0
        && picoui_widget_set_style_class(&dt->widget, props->style_class) != 0) {
        free(dt);
        return 0;
    }
    if (picoui_widget_set_user_data(&dt->widget, props->user_data) != 0) {
        free(dt);
        return 0;
    }
    if (picoui_date_time_set_format(dt, props->format) != 0
        || picoui_date_time_set_date(dt, props->year, props->month, props->day) != 0
        || picoui_date_time_set_time(dt, props->hour, props->minute, props->second) != 0) {
        free(dt);
        return 0;
    }

    return dt;
}

int picoui_date_time_set_format(struct picoui_date_time *dt, const char *format)
{
    if (dt == 0 || format == 0) {
        return -1;
    }

    if (picoui_backend_date_time_set_format(dt, format) != 0) {
        return -1;
    }

    dt->format = format;
    return 0;
}

int picoui_date_time_set_date(struct picoui_date_time *dt, int year, int month, int day)
{
    if (dt == 0 || month < 1 || month > 12 || day < 1 || day > 31) {
        return -1;
    }

    if (picoui_backend_date_time_set_date(dt, year, month, day) != 0) {
        return -1;
    }

    dt->year = year;
    dt->month = month;
    dt->day = day;
    return 0;
}

int picoui_date_time_set_time(struct picoui_date_time *dt, int hour, int minute, int second)
{
    if (dt == 0
        || hour < 0 || hour > 23
        || minute < 0 || minute > 59
        || second < 0 || second > 59) {
        return -1;
    }

    if (picoui_backend_date_time_set_time(dt, hour, minute, second) != 0) {
        return -1;
    }

    dt->hour = hour;
    dt->minute = minute;
    dt->second = second;
    return 0;
}

const char *picoui_date_time_get_format(const struct picoui_date_time *dt)
{
    if (dt == 0) {
        return 0;
    }

    return picoui_backend_date_time_get_format((struct picoui_date_time *)dt);
}
