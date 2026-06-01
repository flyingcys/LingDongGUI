#ifndef PICOUI_DATE_TIME_H
#define PICOUI_DATE_TIME_H

#include "picoui/widget.h"

struct picoui_date_time;

struct picoui_date_time_props {
    const char *id;
    const char *style_class;
    void *user_data;
    const char *format;
    int year;
    int month;
    int day;
    int hour;
    int minute;
    int second;
};

struct picoui_date_time *picoui_date_time_create(struct picoui_widget *parent, const char *id);
struct picoui_date_time *picoui_date_time_create_with_props(
    struct picoui_widget *parent,
    const struct picoui_date_time_props *props);
int picoui_date_time_set_format(struct picoui_date_time *dt, const char *format);
int picoui_date_time_set_date(struct picoui_date_time *dt, int year, int month, int day);
int picoui_date_time_set_time(struct picoui_date_time *dt, int hour, int minute, int second);
const char *picoui_date_time_get_format(const struct picoui_date_time *dt);
int picoui_date_time_get_date(const struct picoui_date_time *dt, int *year, int *month, int *day);
int picoui_date_time_get_time(const struct picoui_date_time *dt, int *hour, int *minute, int *second);

#endif
