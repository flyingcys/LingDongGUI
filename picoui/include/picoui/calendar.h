#ifndef PICOUI_CALENDAR_H
#define PICOUI_CALENDAR_H

#include "picoui/widget.h"

struct picoui_window;
struct picoui_calendar;

struct picoui_calendar_props {
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

struct picoui_calendar *picoui_calendar_create(struct picoui_window *parent, const char *id);
struct picoui_calendar *picoui_calendar_create_with_props(struct picoui_window *parent,
                                                          const struct picoui_calendar_props *props);
int picoui_calendar_set_date(struct picoui_calendar *calendar, int year, int month, int day);
int picoui_calendar_set_day_names(struct picoui_calendar *calendar, const char *const day_names[7]);
int picoui_calendar_get_date(const struct picoui_calendar *calendar,
                             int *year,
                             int *month,
                             int *day);
int picoui_calendar_set_header_visible(struct picoui_calendar *calendar, int visible);
int picoui_calendar_get_header_visible(const struct picoui_calendar *calendar);
int picoui_calendar_set_header_format(struct picoui_calendar *calendar, const char *format);
int picoui_calendar_set_bg_color(struct picoui_calendar *calendar, unsigned int rgb);
int picoui_calendar_set_item_color(struct picoui_calendar *calendar, unsigned int rgb);
int picoui_calendar_set_text_color(struct picoui_calendar *calendar, unsigned int rgb);
const char *picoui_calendar_get_header_format(const struct picoui_calendar *calendar);
int picoui_calendar_get_grid_value(const struct picoui_calendar *calendar, int week, int weekday);
int picoui_calendar_is_current_month_cell(const struct picoui_calendar *calendar, int week, int weekday);

#endif
