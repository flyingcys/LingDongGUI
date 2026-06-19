#ifndef TINYUI_CALENDAR_H
#define TINYUI_CALENDAR_H

#include "widget.h"

struct tinyui_window;
struct tinyui_calendar;

struct tinyui_calendar_props {
    const char *id;
    int year;
    int month;
    int day;
    int width;
    int height;
    int show_header;
    const char *header_format;
    const char *style_class;
    void *user_data;
};

struct tinyui_calendar *tinyui_calendar_create(struct tinyui_window *parent, const char *id);
struct tinyui_calendar *tinyui_calendar_create_with_props(struct tinyui_window *parent,
                                                          const struct tinyui_calendar_props *props);
int tinyui_calendar_set_date(struct tinyui_calendar *calendar, int year, int month, int day);
int tinyui_calendar_set_use_system_date(struct tinyui_calendar *calendar, int enabled);
int tinyui_calendar_set_auto_sys_date(struct tinyui_calendar *calendar, int enabled);
int tinyui_calendar_set_day_names(struct tinyui_calendar *calendar, const char *const day_names[7]);

int tinyui_calendar_get_date(const struct tinyui_calendar *calendar,
                             int *year,
                             int *month,
                             int *day);
int tinyui_calendar_get_use_system_date(const struct tinyui_calendar *calendar);

int tinyui_calendar_set_header_visible(struct tinyui_calendar *calendar, int visible);
int tinyui_calendar_get_header_visible(const struct tinyui_calendar *calendar);
int tinyui_calendar_set_header_format(struct tinyui_calendar *calendar, const char *format);

int tinyui_calendar_set_bg_color(struct tinyui_calendar *calendar, unsigned int rgb);
int tinyui_calendar_set_item_color(struct tinyui_calendar *calendar, unsigned int rgb);
int tinyui_calendar_set_text_color(struct tinyui_calendar *calendar, unsigned int rgb);

const char *tinyui_calendar_get_header_format(const struct tinyui_calendar *calendar);
int tinyui_calendar_get_grid_value(const struct tinyui_calendar *calendar, int week, int weekday);
int tinyui_calendar_is_current_month_cell(const struct tinyui_calendar *calendar, int week, int weekday);

#endif
