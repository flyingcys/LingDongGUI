/*
 * Copyright (c) 2023-2026 flyingcys (flyingcys@gmail.com). All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "internal.h"
#include "widgets/calendar.h"
#include "../../../src/gui/ldCalendar.h"
#include "../../../src/gui/ldBase.h"


extern const arm_2d_a1_font_t ARM_2D_FONT_6x8;
int tinyui_runtime_bridge_bind_leaf_widget(struct tinyui_widget *widget, struct tinyui_app *app);



static void tinyui_calendar_rollback(struct tinyui_calendar *calendar)
{
    if (calendar == 0) {
        return;
    }
    if (calendar->widget.ld_widget != 0) {
        tinyui_widget_destroy_common(&calendar->widget);
    } else {
        ldFree(calendar);
    }
}

static uint8_t s_day_name_0[] = "Su";
static uint8_t s_day_name_1[] = "Mo";
static uint8_t s_day_name_2[] = "Tu";
static uint8_t s_day_name_3[] = "We";
static uint8_t s_day_name_4[] = "Th";
static uint8_t s_day_name_5[] = "Fr";
static uint8_t s_day_name_6[] = "Sa";
static uint8_t *s_day_names[7] = {
    s_day_name_0,
    s_day_name_1,
    s_day_name_2,
    s_day_name_3,
    s_day_name_4,
    s_day_name_5,
    s_day_name_6,
};

static int tinyui_calendar_sync_host_cache(struct tinyui_widget *widget)
{
    struct tinyui_calendar *calendar;
    ldCalendar_t *ld_calendar;
    uint16_t year = 0;
    uint8_t month = 0;
    uint8_t day = 0;
    int index;

    if (widget == NULL) {
        return -1;
    }

    ld_calendar = (ldCalendar_t *)widget->ld_widget;
    if (ld_calendar == NULL) {
        return -1;
    }

    calendar = (struct tinyui_calendar *)widget;
    ldCalendarGetDate(ld_calendar, &year, &month, &day);
    calendar->year = (int)year;
    calendar->month = (int)month;
    calendar->day = (int)day;
    calendar->show_header = ld_calendar->isHeader ? 1 : 0;
    calendar->header_format = (const char *)ld_calendar->headerNameFormat;
    for (index = 0; index < 42; ++index) {
        uint8_t day_num = ld_calendar->calBuf[index];
        calendar->grid_values[index] = (unsigned char)(day_num & 0x7FU);
        calendar->grid_flags[index] = (unsigned char)((day_num & 0x80U) != 0U);
    }
    return 0;
}

static int calendar_props_valid(const struct tinyui_calendar_props *props)
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

static void *tinyui_calendar_ld_init(void *ctx,
                                     struct ld_scene_t *scene,
                                     uint16_t name_id,
                                     uint16_t parent_name_id)
{
    ldCalendar_t *ld_calendar;

    (void)ctx;
    ld_calendar = ldCalendar_init(scene,
                                  NULL,
                                  name_id,
                                  parent_name_id,
                                  0,
                                  0,
                                  280,
                                  180,
                                  (arm_2d_font_t *)&ARM_2D_FONT_6x8,
                                  2026,
                                  6,
                                  15);
    if (ld_calendar == NULL) {
        return 0;
    }

    ldCalendarSetDayNames(ld_calendar, s_day_names);
    ldCalendarSetHeader(ld_calendar, true);
    ldCalendarSetHeaderFormat(ld_calendar, (uint8_t *)"yyyy-mm-dd");
    return ld_calendar;
}

static int tinyui_calendar_set_day_names_ld(struct tinyui_widget *widget, const char *const day_names[7])
{
    ldCalendar_t *ld_calendar = (widget != NULL) ? (ldCalendar_t *)widget->ld_widget : NULL;
    int i;

    if (ld_calendar == NULL || day_names == NULL) {
        return -1;
    }

    for (i = 0; i < 7; ++i) {
        if (day_names[i] == NULL) {
            return -1;
        }
        s_day_names[i] = (uint8_t *)day_names[i];
    }

    ldCalendarSetDayNames(ld_calendar, s_day_names);
    return 0;
}

static int tinyui_calendar_set_date_ld(struct tinyui_widget *widget, int year, int month, int day)
{
    ldCalendar_t *ld_calendar = (widget != NULL) ? (ldCalendar_t *)widget->ld_widget : NULL;

    if (widget == NULL || ld_calendar == NULL || month < 1 || month > 12 || day < 1 || day > 31) {
        return -1;
    }

    ldCalendarSetDate(ld_calendar, (uint16_t)year, (uint8_t)month, (uint8_t)day);
    return tinyui_calendar_sync_host_cache(widget);
}

static int tinyui_calendar_get_date_ld(struct tinyui_widget *widget, int *year, int *month, int *day)
{
    struct tinyui_calendar *calendar;

    if (widget == NULL || tinyui_calendar_sync_host_cache(widget) != 0) {
        return -1;
    }

    calendar = (struct tinyui_calendar *)widget;
    if (year != NULL) {
        *year = calendar->year;
    }
    if (month != NULL) {
        *month = calendar->month;
    }
    if (day != NULL) {
        *day = calendar->day;
    }
    return 0;
}

static int tinyui_calendar_set_header_visible_ld(struct tinyui_widget *widget, int visible)
{
    ldCalendar_t *ld_calendar = (widget != NULL) ? (ldCalendar_t *)widget->ld_widget : NULL;

    if (widget == NULL || ld_calendar == NULL) {
        return -1;
    }

    ldCalendarSetHeader(ld_calendar, visible != 0);
    return tinyui_calendar_sync_host_cache(widget);
}

static int tinyui_calendar_get_header_visible_ld(struct tinyui_widget *widget)
{
    struct tinyui_calendar *calendar;

    if (widget == NULL || tinyui_calendar_sync_host_cache(widget) != 0) {
        return -1;
    }

    calendar = (struct tinyui_calendar *)widget;
    return calendar->show_header;
}

static int tinyui_calendar_set_header_format_ld(struct tinyui_widget *widget, const char *format)
{
    ldCalendar_t *ld_calendar = (widget != NULL) ? (ldCalendar_t *)widget->ld_widget : NULL;

    if (widget == NULL || ld_calendar == NULL || format == NULL) {
        return -1;
    }

    ldCalendarSetHeaderFormat(ld_calendar, (uint8_t *)format);
    return tinyui_calendar_sync_host_cache(widget);
}

static int tinyui_calendar_set_bg_color_ld(struct tinyui_widget *widget, unsigned int rgb)
{
    ldCalendar_t *ld_calendar = (widget != NULL) ? (ldCalendar_t *)widget->ld_widget : NULL;

    if (ld_calendar == NULL || rgb > 0xFFFFFFU) {
        return -1;
    }

    ld_calendar->bgColor = (ldColor)tinyui_rgb_to_ld_color(rgb);
    return 0;
}

static int tinyui_calendar_set_item_color_ld(struct tinyui_widget *widget, unsigned int rgb)
{
    ldCalendar_t *ld_calendar = (widget != NULL) ? (ldCalendar_t *)widget->ld_widget : NULL;

    if (ld_calendar == NULL || rgb > 0xFFFFFFU) {
        return -1;
    }

    ld_calendar->itemColor = (ldColor)tinyui_rgb_to_ld_color(rgb);
    return 0;
}

static int tinyui_calendar_set_text_color_ld(struct tinyui_widget *widget, unsigned int rgb)
{
    ldCalendar_t *ld_calendar = (widget != NULL) ? (ldCalendar_t *)widget->ld_widget : NULL;

    if (ld_calendar == NULL || rgb > 0xFFFFFFU) {
        return -1;
    }

    ld_calendar->textColor = (ldColor)tinyui_rgb_to_ld_color(rgb);
    return 0;
}

static int tinyui_calendar_set_use_system_date_ld(struct tinyui_widget *widget, int enabled)
{
    ldCalendar_t *ld_calendar = (widget != NULL) ? (ldCalendar_t *)widget->ld_widget : NULL;

    if (widget == NULL || ld_calendar == NULL) {
        return -1;
    }

    ldCalendarSetAutoSysDate(ld_calendar, enabled != 0);
    if (enabled != 0) {
        ldCalendar_on_frame_start(widget->owner != NULL ? widget->owner->ld_scene : NULL, ld_calendar);
    }
    return tinyui_calendar_sync_host_cache(widget);
}

static int tinyui_calendar_get_use_system_date_ld(struct tinyui_widget *widget, int *enabled)
{
    ldCalendar_t *ld_calendar = (widget != NULL) ? (ldCalendar_t *)widget->ld_widget : NULL;

    if (ld_calendar == NULL || enabled == NULL) {
        return -1;
    }

    *enabled = ld_calendar->isAutoSysDate ? 1 : 0;
    return 0;
}

static const char *tinyui_calendar_get_header_format_ld(struct tinyui_widget *widget)
{
    struct tinyui_calendar *calendar;

    if (widget == NULL || tinyui_calendar_sync_host_cache(widget) != 0) {
        return NULL;
    }

    calendar = (struct tinyui_calendar *)widget;
    return calendar->header_format;
}

static int tinyui_calendar_get_grid_value_ld(struct tinyui_widget *widget, int week, int weekday)
{
    struct tinyui_calendar *calendar;
    int index;

    if (widget == NULL || week < 0 || week >= 6 || weekday < 0 || weekday >= 7 ||
        tinyui_calendar_sync_host_cache(widget) != 0) {
        return -1;
    }

    calendar = (struct tinyui_calendar *)widget;
    index = week * 7 + weekday;
    return (int)calendar->grid_values[index];
}

static int tinyui_calendar_is_current_month_cell_ld(struct tinyui_widget *widget, int week, int weekday)
{
    struct tinyui_calendar *calendar;
    int index;

    if (widget == NULL || week < 0 || week >= 6 || weekday < 0 || weekday >= 7 ||
        tinyui_calendar_sync_host_cache(widget) != 0) {
        return -1;
    }

    calendar = (struct tinyui_calendar *)widget;
    index = week * 7 + weekday;
    return calendar->grid_flags[index] != 0 ? 1 : 0;
}

static int calendar_sync_grid_local(struct tinyui_calendar *calendar)
{
    int week;
    int weekday;

    if (calendar == 0) {
        return -1;
    }

    for (week = 0; week < 6; ++week) {
        for (weekday = 0; weekday < 7; ++weekday) {
            int index = week * 7 + weekday;
            int day = tinyui_calendar_get_grid_value_ld(&calendar->widget, week, weekday);
            int current = tinyui_calendar_is_current_month_cell_ld(&calendar->widget,
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

struct tinyui_calendar *tinyui_calendar_create(struct tinyui_window *parent, const char *id)
{
    struct tinyui_calendar *calendar;

    if (parent == 0 || id == 0) {
        return 0;
    }
    calendar = (struct tinyui_calendar *)tinyui_widget_create_leaf(&parent->widget,
                                                                   TINYUI_BACKEND_WIDGET_CALENDAR,
                                                                   tinyui_calendar_ld_init,
                                                                   0,
                                                                   sizeof(*calendar));
    if (calendar == 0) {
        return 0;
    }
    calendar->id = id;

    if (tinyui_calendar_set_date(calendar, 2026, 6, 15) != 0 ||
        tinyui_calendar_set_header_visible(calendar, 1) != 0 ||
        tinyui_calendar_set_header_format(calendar, "yyyy-mm-dd") != 0) {
        tinyui_calendar_rollback(calendar);
        return 0;
    }
    return calendar;
}

struct tinyui_calendar *tinyui_calendar_create_with_props(struct tinyui_window *parent,
                                                          const struct tinyui_calendar_props *props)
{
    struct tinyui_calendar *calendar;

    if (!calendar_props_valid(props)) {
        return 0;
    }

    calendar = tinyui_calendar_create(parent, props->id);
    if (calendar == 0) {
        return 0;
    }

    if (tinyui_widget_set_user_data(&calendar->widget, props->user_data) != 0) {
        tinyui_calendar_rollback(calendar);
        return 0;
    }
    if (props->style_class != 0 &&
        tinyui_widget_set_style_class(&calendar->widget, props->style_class) != 0) {
        tinyui_calendar_rollback(calendar);
        return 0;
    }
    if ((props->width > 0 || props->height > 0) &&
        tinyui_widget_set_size(&calendar->widget, props->width, props->height) != 0) {
        tinyui_calendar_rollback(calendar);
        return 0;
    }
    if (tinyui_calendar_set_date(calendar, props->year, props->month, props->day) != 0 ||
        tinyui_calendar_set_header_visible(calendar, props->show_header) != 0 ||
        tinyui_calendar_set_header_format(calendar, props->header_format) != 0) {
        tinyui_calendar_rollback(calendar);
        return 0;
    }

    return calendar;
}

int tinyui_calendar_set_date(struct tinyui_calendar *calendar, int year, int month, int day)
{
    if (calendar == 0 || month < 1 || month > 12 || day < 1 || day > 31) {
        return -1;
    }

    if (tinyui_calendar_set_date_ld(&calendar->widget, year, month, day) != 0) {
        return -1;
    }

    calendar->year = year;
    calendar->month = month;
    calendar->day = day;
    return calendar_sync_grid_local(calendar);
}

int tinyui_calendar_set_use_system_date(struct tinyui_calendar *calendar, int enabled)
{
    if (calendar == 0) {
        return -1;
    }

    if (tinyui_calendar_set_use_system_date_ld(&calendar->widget, enabled) != 0) {
        return -1;
    }

    calendar->use_system_date = enabled != 0;
    return calendar_sync_grid_local(calendar);
}

int tinyui_calendar_set_auto_sys_date(struct tinyui_calendar *calendar, int enabled)
{
    return tinyui_calendar_set_use_system_date(calendar, enabled);
}

int tinyui_calendar_set_day_names(struct tinyui_calendar *calendar, const char *const day_names[7])
{
    if (calendar == 0 || day_names == 0) {
        return -1;
    }

    return tinyui_calendar_set_day_names_ld(&calendar->widget, day_names);
}

int tinyui_calendar_get_date(const struct tinyui_calendar *calendar, int *year, int *month, int *day)
{
    struct tinyui_calendar *mutable_calendar = (struct tinyui_calendar *)calendar;

    if (calendar == 0) {
        return -1;
    }

    if (tinyui_calendar_get_date_ld(&mutable_calendar->widget, year, month, day) != 0) {
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

int tinyui_calendar_get_use_system_date(const struct tinyui_calendar *calendar)
{
    int enabled = 0;

    if (calendar == 0) {
        return -1;
    }

    if (tinyui_calendar_get_use_system_date_ld((struct tinyui_widget *)&calendar->widget, &enabled) != 0) {
        return -1;
    }

    ((struct tinyui_calendar *)calendar)->use_system_date = enabled;
    return enabled;
}

int tinyui_calendar_set_header_visible(struct tinyui_calendar *calendar, int visible)
{
    if (calendar == 0) {
        return -1;
    }

    if (tinyui_calendar_set_header_visible_ld(&calendar->widget, visible != 0) != 0) {
        return -1;
    }
    calendar->show_header = visible != 0;
    return 0;
}

int tinyui_calendar_get_header_visible(const struct tinyui_calendar *calendar)
{
    int visible;

    if (calendar == 0) {
        return -1;
    }

    visible = tinyui_calendar_get_header_visible_ld((struct tinyui_widget *)&calendar->widget);
    if (visible >= 0) {
        ((struct tinyui_calendar *)calendar)->show_header = visible;
    }
    return visible;
}

int tinyui_calendar_set_header_format(struct tinyui_calendar *calendar, const char *format)
{
    if (calendar == 0 || format == 0) {
        return -1;
    }

    if (tinyui_calendar_set_header_format_ld(&calendar->widget, format) != 0) {
        return -1;
    }
    calendar->header_format = tinyui_calendar_get_header_format_ld(&calendar->widget);
    return calendar->header_format != 0 ? 0 : -1;
}

int tinyui_calendar_set_bg_color(struct tinyui_calendar *calendar, unsigned int rgb)
{
    if (calendar == 0 || rgb > 0xFFFFFFU) {
        return -1;
    }

    return tinyui_calendar_set_bg_color_ld(&calendar->widget, rgb);
}

int tinyui_calendar_set_item_color(struct tinyui_calendar *calendar, unsigned int rgb)
{
    if (calendar == 0 || rgb > 0xFFFFFFU) {
        return -1;
    }

    return tinyui_calendar_set_item_color_ld(&calendar->widget, rgb);
}

int tinyui_calendar_set_text_color(struct tinyui_calendar *calendar, unsigned int rgb)
{
    if (calendar == 0 || rgb > 0xFFFFFFU) {
        return -1;
    }

    return tinyui_calendar_set_text_color_ld(&calendar->widget, rgb);
}

const char *tinyui_calendar_get_header_format(const struct tinyui_calendar *calendar)
{
    const char *format;

    if (calendar == 0) {
        return 0;
    }

    format = tinyui_calendar_get_header_format_ld((struct tinyui_widget *)&calendar->widget);
    if (format != 0) {
        ((struct tinyui_calendar *)calendar)->header_format = format;
    }
    return format;
}

int tinyui_calendar_get_grid_value(const struct tinyui_calendar *calendar, int week, int weekday)
{
    int day;
    int index;

    if (calendar == 0 || week < 0 || week >= 6 || weekday < 0 || weekday >= 7) {
        return -1;
    }

    day = tinyui_calendar_get_grid_value_ld((struct tinyui_widget *)&calendar->widget, week, weekday);
    if (day >= 0) {
        index = week * 7 + weekday;
        ((struct tinyui_calendar *)calendar)->grid_values[index] = (unsigned char)day;
    }
    return day;
}

int tinyui_calendar_is_current_month_cell(const struct tinyui_calendar *calendar, int week, int weekday)
{
    int current;
    int index;

    if (calendar == 0 || week < 0 || week >= 6 || weekday < 0 || weekday >= 7) {
        return -1;
    }

    current = tinyui_calendar_is_current_month_cell_ld((struct tinyui_widget *)&calendar->widget,
                                                            week,
                                                            weekday);
    if (current >= 0) {
        index = week * 7 + weekday;
        ((struct tinyui_calendar *)calendar)->grid_flags[index] = (unsigned char)current;
    }
    return current;
}
