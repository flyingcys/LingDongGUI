#ifndef TINYUI_DATE_TIME_H
#define TINYUI_DATE_TIME_H

#include "widget.h"

struct tinyui_date_time;

struct tinyui_date_time_props {
    const char *id;
    const char *style_class;
    void *user_data;
    const char *format;
    unsigned int text_color;
    unsigned int bg_color;
    enum tinyui_align align;
    int transparent;
    int year;
    int month;
    int day;
    int hour;
    int minute;
    int second;
};

struct tinyui_date_time *tinyui_date_time_create(struct tinyui_widget *parent, const char *id);

struct tinyui_date_time *tinyui_date_time_create_with_props(
    struct tinyui_widget *parent,
    const struct tinyui_date_time_props *props);

struct tinyui_date_time *tinyui_date_time_init(struct tinyui_widget *parent, const char *id);

int tinyui_date_time_set_format(struct tinyui_date_time *dt, const char *format);

int tinyui_date_time_set_date(struct tinyui_date_time *dt, int year, int month, int day);

int tinyui_date_time_set_time(struct tinyui_date_time *dt, int hour, int minute, int second);

int tinyui_date_time_set_text_color(struct tinyui_date_time *dt, unsigned int rgb);

int tinyui_date_time_set_background_color(struct tinyui_date_time *dt, unsigned int rgb);

int tinyui_date_time_set_bg_color(struct tinyui_date_time *dt, unsigned int rgb);

int tinyui_date_time_set_align(struct tinyui_date_time *dt, enum tinyui_align align);

int tinyui_date_time_set_transparent(struct tinyui_date_time *dt, int transparent);

int tinyui_date_time_set_use_system_time(struct tinyui_date_time *dt, int enabled);

const char *tinyui_date_time_get_format(const struct tinyui_date_time *dt);

int tinyui_date_time_get_date(const struct tinyui_date_time *dt, int *year, int *month, int *day);

int tinyui_date_time_get_time(const struct tinyui_date_time *dt, int *hour, int *minute, int *second);

int tinyui_date_time_get_transparent(const struct tinyui_date_time *dt);

int tinyui_date_time_get_use_system_time(const struct tinyui_date_time *dt);

#endif
