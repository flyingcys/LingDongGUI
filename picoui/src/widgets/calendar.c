#include "internal.h"
#include "picoui/calendar.h"

#include <stdlib.h>

static int picoui_calendar_props_are_valid(const struct picoui_calendar_props *props)
{
    return props != 0 &&
           props->id != 0 &&
           props->month >= 1 &&
           props->month <= 12 &&
           props->day >= 1 &&
           props->day <= 31 &&
           props->width >= 0 &&
           props->height >= 0 &&
           props->header_format != 0;
}

static int picoui_calendar_sync_grid(struct picoui_calendar *calendar)
{
    int week;
    int weekday;

    if (calendar == 0) {
        return -1;
    }

    for (week = 0; week < 6; ++week) {
        for (weekday = 0; weekday < 7; ++weekday) {
            int index = week * 7 + weekday;
            int day = picoui_backend_calendar_get_grid_value(calendar->widget.backend_widget, week, weekday);
            int current = picoui_backend_calendar_is_current_month_cell(calendar->widget.backend_widget,
                                                                        week,
                                                                        weekday);
            if (day < 0 || current < 0) {
                return -1;
            }
            calendar->grid_values[index] = (unsigned char)day;
            calendar->grid_flags[index] = (unsigned char)current;
        }
    }

    return 0;
}

struct picoui_calendar *picoui_calendar_create(struct picoui_window *parent, const char *id)
{
    struct picoui_calendar *calendar;

    if (parent == 0 || id == 0) {
        return 0;
    }

    calendar = calloc(1, sizeof(*calendar));
    if (calendar == 0) {
        return 0;
    }

    calendar->widget.backend_widget =
        picoui_backend_create_calendar(parent->widget.backend_widget, id);
    if (calendar->widget.backend_widget == 0) {
        free(calendar);
        return 0;
    }

    calendar->id = id;
    calendar->widget.visible = 1;
    calendar->widget.enabled = 1;
    if (picoui_backend_widget_bind_host(calendar->widget.backend_widget, &calendar->widget) != 0) {
        free(calendar);
        return 0;
    }
    if (picoui_calendar_set_date(calendar, 2026, 6, 15) != 0 ||
        picoui_calendar_set_header_visible(calendar, 1) != 0 ||
        picoui_calendar_set_header_format(calendar, "yyyy-mm-dd") != 0) {
        free(calendar);
        return 0;
    }
    return calendar;
}

struct picoui_calendar *picoui_calendar_create_with_props(struct picoui_window *parent,
                                                          const struct picoui_calendar_props *props)
{
    struct picoui_calendar *calendar;

    if (!picoui_calendar_props_are_valid(props)) {
        return 0;
    }

    calendar = picoui_calendar_create(parent, props->id);
    if (calendar == 0) {
        return 0;
    }

    if (picoui_widget_set_user_data(&calendar->widget, props->user_data) != 0) {
        free(calendar);
        return 0;
    }
    if (props->style_class != 0 &&
        picoui_widget_set_style_class(&calendar->widget, props->style_class) != 0) {
        free(calendar);
        return 0;
    }
    if ((props->width > 0 || props->height > 0) &&
        picoui_widget_set_size(&calendar->widget, props->width, props->height) != 0) {
        free(calendar);
        return 0;
    }
    if (picoui_calendar_set_date(calendar, props->year, props->month, props->day) != 0 ||
        picoui_calendar_set_header_visible(calendar, props->show_header) != 0 ||
        picoui_calendar_set_header_format(calendar, props->header_format) != 0) {
        free(calendar);
        return 0;
    }

    return calendar;
}

int picoui_calendar_set_date(struct picoui_calendar *calendar, int year, int month, int day)
{
    if (calendar == 0 || month < 1 || month > 12 || day < 1 || day > 31) {
        return -1;
    }

    if (picoui_backend_calendar_set_date(calendar->widget.backend_widget, year, month, day) != 0) {
        return -1;
    }

    calendar->year = year;
    calendar->month = month;
    calendar->day = day;
    return picoui_calendar_sync_grid(calendar);
}

int picoui_calendar_set_day_names(struct picoui_calendar *calendar, const char *const day_names[7])
{
    if (calendar == 0 || day_names == 0) {
        return -1;
    }

    return picoui_backend_calendar_set_day_names(calendar->widget.backend_widget, day_names);
}

int picoui_calendar_get_date(const struct picoui_calendar *calendar, int *year, int *month, int *day)
{
    struct picoui_calendar *mutable_calendar = (struct picoui_calendar *)calendar;

    if (calendar == 0) {
        return -1;
    }

    if (picoui_backend_calendar_get_date(mutable_calendar->widget.backend_widget, year, month, day) != 0) {
        return -1;
    }
    if (year != 0) {
        mutable_calendar->year = *year;
    }
    if (month != 0) {
        mutable_calendar->month = *month;
    }
    if (day != 0) {
        mutable_calendar->day = *day;
    }
    return 0;
}

int picoui_calendar_set_header_visible(struct picoui_calendar *calendar, int visible)
{
    if (calendar == 0) {
        return -1;
    }

    if (picoui_backend_calendar_set_header_visible(calendar->widget.backend_widget, visible != 0) != 0) {
        return -1;
    }
    calendar->show_header = visible != 0;
    return 0;
}

int picoui_calendar_get_header_visible(const struct picoui_calendar *calendar)
{
    int visible;

    if (calendar == 0) {
        return -1;
    }

    visible = picoui_backend_calendar_get_header_visible((void *)calendar->widget.backend_widget);
    if (visible >= 0) {
        ((struct picoui_calendar *)calendar)->show_header = visible;
    }
    return visible;
}

int picoui_calendar_set_header_format(struct picoui_calendar *calendar, const char *format)
{
    if (calendar == 0 || format == 0) {
        return -1;
    }

    if (picoui_backend_calendar_set_header_format(calendar->widget.backend_widget, format) != 0) {
        return -1;
    }
    calendar->header_format = picoui_backend_calendar_get_header_format(calendar->widget.backend_widget);
    return calendar->header_format != 0 ? 0 : -1;
}

int picoui_calendar_set_bg_color(struct picoui_calendar *calendar, unsigned int rgb)
{
    if (calendar == 0 || rgb > 0xFFFFFFU) {
        return -1;
    }

    return picoui_backend_calendar_set_bg_color(calendar->widget.backend_widget, rgb);
}

int picoui_calendar_set_item_color(struct picoui_calendar *calendar, unsigned int rgb)
{
    if (calendar == 0 || rgb > 0xFFFFFFU) {
        return -1;
    }

    return picoui_backend_calendar_set_item_color(calendar->widget.backend_widget, rgb);
}

int picoui_calendar_set_text_color(struct picoui_calendar *calendar, unsigned int rgb)
{
    if (calendar == 0 || rgb > 0xFFFFFFU) {
        return -1;
    }

    return picoui_backend_calendar_set_text_color(calendar->widget.backend_widget, rgb);
}

const char *picoui_calendar_get_header_format(const struct picoui_calendar *calendar)
{
    const char *format;

    if (calendar == 0) {
        return 0;
    }

    format = picoui_backend_calendar_get_header_format((void *)calendar->widget.backend_widget);
    if (format != 0) {
        ((struct picoui_calendar *)calendar)->header_format = format;
    }
    return format;
}

int picoui_calendar_get_grid_value(const struct picoui_calendar *calendar, int week, int weekday)
{
    int day;
    int index;

    if (calendar == 0 || week < 0 || week >= 6 || weekday < 0 || weekday >= 7) {
        return -1;
    }

    day = picoui_backend_calendar_get_grid_value((void *)calendar->widget.backend_widget, week, weekday);
    if (day >= 0) {
        index = week * 7 + weekday;
        ((struct picoui_calendar *)calendar)->grid_values[index] = (unsigned char)day;
    }
    return day;
}

int picoui_calendar_is_current_month_cell(const struct picoui_calendar *calendar, int week, int weekday)
{
    int current;
    int index;

    if (calendar == 0 || week < 0 || week >= 6 || weekday < 0 || weekday >= 7) {
        return -1;
    }

    current = picoui_backend_calendar_is_current_month_cell((void *)calendar->widget.backend_widget,
                                                            week,
                                                            weekday);
    if (current >= 0) {
        index = week * 7 + weekday;
        ((struct picoui_calendar *)calendar)->grid_flags[index] = (unsigned char)current;
    }
    return current;
}
