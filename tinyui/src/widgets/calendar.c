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
#include "calendar.h"

#include "../core/runtime_bridge.h"
#include "../../../src/gui/ldCalendar.h"

#include <stdlib.h>

extern const arm_2d_a1_font_t ARM_2D_FONT_6x8;

int tinyui_runtime_bridge_unbind_host(void *backend_widget);
int tinyui_runtime_bridge_detach_from_parent(void *backend_widget);

static uint8_t g_tinyui_calendar_day_name_0[] = "Su";
static uint8_t g_tinyui_calendar_day_name_1[] = "Mo";
static uint8_t g_tinyui_calendar_day_name_2[] = "Tu";
static uint8_t g_tinyui_calendar_day_name_3[] = "We";
static uint8_t g_tinyui_calendar_day_name_4[] = "Th";
static uint8_t g_tinyui_calendar_day_name_5[] = "Fr";
static uint8_t g_tinyui_calendar_day_name_6[] = "Sa";
static uint8_t *g_tinyui_calendar_day_names[7] = {
    g_tinyui_calendar_day_name_0,
    g_tinyui_calendar_day_name_1,
    g_tinyui_calendar_day_name_2,
    g_tinyui_calendar_day_name_3,
    g_tinyui_calendar_day_name_4,
    g_tinyui_calendar_day_name_5,
    g_tinyui_calendar_day_name_6,
};

static ldColor tinyui_calendar_rgb_to_ld(unsigned int rgb)
{
    return (ldColor)rgb;
}

static ldCalendar_t *tinyui_calendar_get_ld(void *backend_widget)
{
    struct picoui_backend_widget *widget = backend_widget;

    if (widget == NULL || widget->ld_widget == NULL) {
        return NULL;
    }

    return (ldCalendar_t *)widget->ld_widget;
}

static int tinyui_calendar_sync_host_cache(struct picoui_backend_widget *backend)
{
    struct picoui_calendar *calendar;
    ldCalendar_t *ld_calendar;
    uint16_t year = 0;
    uint8_t month = 0;
    uint8_t day = 0;
    int index;

    if (backend == NULL || backend->host_widget == NULL) {
        return -1;
    }

    ld_calendar = tinyui_calendar_get_ld(backend);
    if (ld_calendar == NULL) {
        return -1;
    }

    calendar = (struct picoui_calendar *)backend->host_widget;
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

static void *tinyui_calendar_create_backend_local(void *parent, const char *id)
{
    struct picoui_backend_widget *widget;
    struct picoui_backend_widget *parent_widget = parent;
    struct picoui_backend_app_state *app_state;
    ldCalendar_t *ld_calendar;
    uint16_t name_id;

    if (parent == 0 || id == 0) {
        return 0;
    }

    app_state = tinyui_runtime_bridge_backend_state_from_parent(parent);
    if (app_state == NULL || app_state->ld_scene == NULL || parent_widget->ld_widget == NULL) {
        return 0;
    }

    widget = calloc(1, sizeof(*widget));
    if (widget == 0) {
        return 0;
    }

    name_id = tinyui_runtime_bridge_next_name_id(parent);
    if (name_id == 0) {
        free(widget);
        return 0;
    }
    ld_calendar = ldCalendar_init(app_state->ld_scene,
                                  NULL,
                                  name_id,
                                  parent_widget->ld_name_id,
                                  0,
                                  0,
                                  280,
                                  180,
                                  (arm_2d_font_t *)&ARM_2D_FONT_6x8,
                                  2026,
                                  6,
                                  15);
    if (ld_calendar == NULL) {
        free(widget);
        return 0;
    }

    ldCalendarSetDayNames(ld_calendar, g_tinyui_calendar_day_names);
    ldCalendarSetHeader(ld_calendar, true);
    ldCalendarSetHeaderFormat(ld_calendar, (uint8_t *)"yyyy-mm-dd");
    if (tinyui_widget_init_child(widget,
                                         parent,
                                         PICOUI_BACKEND_WIDGET_CALENDAR,
                                         id,
                                         parent_widget->theme) != 0) {
        ldCalendar_depose(app_state->ld_scene, ld_calendar);
        free(widget);
        return 0;
    }
    widget->ld_widget = ld_calendar;
    widget->ld_name_id = name_id;
    if (tinyui_widget_attach_child(parent, widget) != 0) {
        ldCalendar_depose(app_state->ld_scene, ld_calendar);
        free(widget);
        return 0;
    }
    return widget;
}

static int tinyui_calendar_props_are_valid(const struct picoui_calendar_props *props)
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

static int tinyui_calendar_set_day_names(void *backend_widget, const char *const day_names[7])
{
    ldCalendar_t *ld_calendar = tinyui_calendar_get_ld(backend_widget);
    int i;

    if (ld_calendar == NULL || day_names == NULL) {
        return -1;
    }

    for (i = 0; i < 7; ++i) {
        if (day_names[i] == NULL) {
            return -1;
        }
        g_tinyui_calendar_day_names[i] = (uint8_t *)day_names[i];
    }

    ldCalendarSetDayNames(ld_calendar, g_tinyui_calendar_day_names);
    return 0;
}

static int tinyui_calendar_set_date(void *backend_widget, int year, int month, int day)
{
    struct picoui_backend_widget *backend = backend_widget;
    ldCalendar_t *ld_calendar = tinyui_calendar_get_ld(backend_widget);

    if (backend == NULL || ld_calendar == NULL || month < 1 || month > 12 || day < 1 || day > 31) {
        return -1;
    }

    ldCalendarSetDate(ld_calendar, (uint16_t)year, (uint8_t)month, (uint8_t)day);
    return tinyui_calendar_sync_host_cache(backend);
}

static int tinyui_calendar_get_date(void *backend_widget, int *year, int *month, int *day)
{
    struct picoui_backend_widget *backend = backend_widget;
    struct picoui_calendar *calendar;

    if (backend == NULL || tinyui_calendar_sync_host_cache(backend) != 0) {
        return -1;
    }

    calendar = (struct picoui_calendar *)backend->host_widget;
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

static int tinyui_calendar_set_header_visible(void *backend_widget, int visible)
{
    struct picoui_backend_widget *backend = backend_widget;
    ldCalendar_t *ld_calendar = tinyui_calendar_get_ld(backend_widget);

    if (backend == NULL || ld_calendar == NULL) {
        return -1;
    }

    ldCalendarSetHeader(ld_calendar, visible != 0);
    return tinyui_calendar_sync_host_cache(backend);
}

static int tinyui_calendar_get_header_visible(void *backend_widget)
{
    struct picoui_backend_widget *backend = backend_widget;
    struct picoui_calendar *calendar;

    if (backend == NULL || tinyui_calendar_sync_host_cache(backend) != 0) {
        return -1;
    }

    calendar = (struct picoui_calendar *)backend->host_widget;
    return calendar->show_header;
}

static int tinyui_calendar_set_header_format(void *backend_widget, const char *format)
{
    struct picoui_backend_widget *backend = backend_widget;
    ldCalendar_t *ld_calendar = tinyui_calendar_get_ld(backend_widget);

    if (backend == NULL || ld_calendar == NULL || format == NULL) {
        return -1;
    }

    ldCalendarSetHeaderFormat(ld_calendar, (uint8_t *)format);
    return tinyui_calendar_sync_host_cache(backend);
}

static int tinyui_calendar_set_bg_color(void *backend_widget, unsigned int rgb)
{
    ldCalendar_t *ld_calendar = tinyui_calendar_get_ld(backend_widget);

    if (ld_calendar == NULL || rgb > 0xFFFFFFU) {
        return -1;
    }

    ld_calendar->bgColor = tinyui_calendar_rgb_to_ld(rgb);
    return 0;
}

static int tinyui_calendar_set_item_color(void *backend_widget, unsigned int rgb)
{
    ldCalendar_t *ld_calendar = tinyui_calendar_get_ld(backend_widget);

    if (ld_calendar == NULL || rgb > 0xFFFFFFU) {
        return -1;
    }

    ld_calendar->itemColor = tinyui_calendar_rgb_to_ld(rgb);
    return 0;
}

static int tinyui_calendar_set_text_color(void *backend_widget, unsigned int rgb)
{
    ldCalendar_t *ld_calendar = tinyui_calendar_get_ld(backend_widget);

    if (ld_calendar == NULL || rgb > 0xFFFFFFU) {
        return -1;
    }

    ld_calendar->textColor = tinyui_calendar_rgb_to_ld(rgb);
    return 0;
}

static int tinyui_calendar_set_use_system_date(void *backend_widget, int enabled)
{
    struct picoui_backend_widget *backend = backend_widget;
    ldCalendar_t *ld_calendar = tinyui_calendar_get_ld(backend_widget);

    if (backend == NULL || ld_calendar == NULL) {
        return -1;
    }

    ldCalendarSetAutoSysDate(ld_calendar, enabled != 0);
    if (enabled != 0) {
        ldCalendar_on_frame_start(NULL, ld_calendar);
    }
    return tinyui_calendar_sync_host_cache(backend);
}

static int tinyui_calendar_get_use_system_date(void *backend_widget, int *enabled)
{
    ldCalendar_t *ld_calendar = tinyui_calendar_get_ld(backend_widget);

    if (ld_calendar == NULL || enabled == NULL) {
        return -1;
    }

    *enabled = ld_calendar->isAutoSysDate ? 1 : 0;
    return 0;
}

static const char *tinyui_calendar_get_header_format(void *backend_widget)
{
    struct picoui_backend_widget *backend = backend_widget;
    struct picoui_calendar *calendar;

    if (backend == NULL || tinyui_calendar_sync_host_cache(backend) != 0) {
        return NULL;
    }

    calendar = (struct picoui_calendar *)backend->host_widget;
    return calendar->header_format;
}

static int tinyui_calendar_get_grid_value(void *backend_widget, int week, int weekday)
{
    struct picoui_backend_widget *backend = backend_widget;
    struct picoui_calendar *calendar;
    int index;

    if (backend == NULL || week < 0 || week >= 6 || weekday < 0 || weekday >= 7 ||
        tinyui_calendar_sync_host_cache(backend) != 0) {
        return -1;
    }

    calendar = (struct picoui_calendar *)backend->host_widget;
    index = week * 7 + weekday;
    return (int)calendar->grid_values[index];
}

static int tinyui_calendar_is_current_month_cell(void *backend_widget, int week, int weekday)
{
    struct picoui_backend_widget *backend = backend_widget;
    struct picoui_calendar *calendar;
    int index;

    if (backend == NULL || week < 0 || week >= 6 || weekday < 0 || weekday >= 7 ||
        tinyui_calendar_sync_host_cache(backend) != 0) {
        return -1;
    }

    calendar = (struct picoui_calendar *)backend->host_widget;
    index = week * 7 + weekday;
    return calendar->grid_flags[index] != 0 ? 1 : 0;
}

static int tinyui_calendar_sync_grid(struct picoui_calendar *calendar)
{
    int week;
    int weekday;

    if (calendar == 0) {
        return -1;
    }

    for (week = 0; week < 6; ++week) {
        for (weekday = 0; weekday < 7; ++weekday) {
            int index = week * 7 + weekday;
            int day = tinyui_calendar_get_grid_value(calendar->widget.backend_widget, week, weekday);
            int current = tinyui_calendar_is_current_month_cell(calendar->widget.backend_widget,
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

static void tinyui_calendar_dispose_partial(struct picoui_calendar *calendar)
{
    struct picoui_backend_widget *backend;
    struct picoui_backend_app_state *app_state;

    if (calendar == 0) {
        return;
    }

    backend = (struct picoui_backend_widget *)calendar->widget.backend_widget;
    if (backend != 0) {
        app_state = tinyui_runtime_bridge_backend_state(backend->owner);
        if (backend->parent != 0) {
            (void)tinyui_runtime_bridge_detach_from_parent(backend);
        }
        (void)tinyui_runtime_bridge_unbind_host(backend);
        if (app_state != 0 && app_state->ld_scene != 0 && backend->ld_widget != 0) {
            ldCalendar_depose(app_state->ld_scene, (ldCalendar_t *)backend->ld_widget);
        }
        free(backend);
    }

    free(calendar);
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

    calendar->widget.backend_widget = tinyui_calendar_create_backend_local(parent->widget.backend_widget, id);
    if (calendar->widget.backend_widget == 0) {
        free(calendar);
        return 0;
    }

    calendar->id = id;
    calendar->widget.visible = 1;
    calendar->widget.enabled = 1;
    if (tinyui_runtime_bridge_bind_host(calendar->widget.backend_widget, &calendar->widget) != 0) {
        tinyui_calendar_dispose_partial(calendar);
        return 0;
    }
    if (picoui_calendar_set_date(calendar, 2026, 6, 15) != 0 ||
        picoui_calendar_set_header_visible(calendar, 1) != 0 ||
        picoui_calendar_set_header_format(calendar, "yyyy-mm-dd") != 0) {
        tinyui_calendar_dispose_partial(calendar);
        return 0;
    }
    return calendar;
}

struct picoui_calendar *picoui_calendar_init(struct picoui_window *parent, const char *id)
{
    return picoui_calendar_create(parent, id);
}

struct picoui_calendar *picoui_calendar_create_with_props(struct picoui_window *parent,
                                                          const struct picoui_calendar_props *props)
{
    struct picoui_calendar *calendar;

    if (!tinyui_calendar_props_are_valid(props)) {
        return 0;
    }

    calendar = picoui_calendar_create(parent, props->id);
    if (calendar == 0) {
        return 0;
    }

    if (picoui_widget_set_user_data(&calendar->widget, props->user_data) != 0) {
        tinyui_calendar_dispose_partial(calendar);
        return 0;
    }
    if (props->style_class != 0 &&
        picoui_widget_set_style_class(&calendar->widget, props->style_class) != 0) {
        tinyui_calendar_dispose_partial(calendar);
        return 0;
    }
    if ((props->width > 0 || props->height > 0) &&
        picoui_widget_set_size(&calendar->widget, props->width, props->height) != 0) {
        tinyui_calendar_dispose_partial(calendar);
        return 0;
    }
    if (picoui_calendar_set_date(calendar, props->year, props->month, props->day) != 0 ||
        picoui_calendar_set_header_visible(calendar, props->show_header) != 0 ||
        picoui_calendar_set_header_format(calendar, props->header_format) != 0) {
        tinyui_calendar_dispose_partial(calendar);
        return 0;
    }

    return calendar;
}

int picoui_calendar_set_date(struct picoui_calendar *calendar, int year, int month, int day)
{
    if (calendar == 0 || month < 1 || month > 12 || day < 1 || day > 31) {
        return -1;
    }

    if (tinyui_calendar_set_date(calendar->widget.backend_widget, year, month, day) != 0) {
        return -1;
    }

    calendar->year = year;
    calendar->month = month;
    calendar->day = day;
    return tinyui_calendar_sync_grid(calendar);
}

int picoui_calendar_set_use_system_date(struct picoui_calendar *calendar, int enabled)
{
    if (calendar == 0) {
        return -1;
    }

    if (tinyui_calendar_set_use_system_date(calendar->widget.backend_widget, enabled) != 0) {
        return -1;
    }

    calendar->use_system_date = enabled != 0;
    return tinyui_calendar_sync_grid(calendar);
}

int picoui_calendar_set_day_names(struct picoui_calendar *calendar, const char *const day_names[7])
{
    if (calendar == 0 || day_names == 0) {
        return -1;
    }

    return tinyui_calendar_set_day_names(calendar->widget.backend_widget, day_names);
}

int picoui_calendar_get_date(const struct picoui_calendar *calendar, int *year, int *month, int *day)
{
    struct picoui_calendar *mutable_calendar = (struct picoui_calendar *)calendar;

    if (calendar == 0) {
        return -1;
    }

    if (tinyui_calendar_get_date(mutable_calendar->widget.backend_widget, year, month, day) != 0) {
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

int picoui_calendar_get_use_system_date(const struct picoui_calendar *calendar)
{
    int enabled = 0;

    if (calendar == 0) {
        return -1;
    }

    if (tinyui_calendar_get_use_system_date((void *)calendar->widget.backend_widget, &enabled) != 0) {
        return -1;
    }

    ((struct picoui_calendar *)calendar)->use_system_date = enabled;
    return enabled;
}

int picoui_calendar_set_header_visible(struct picoui_calendar *calendar, int visible)
{
    if (calendar == 0) {
        return -1;
    }

    if (tinyui_calendar_set_header_visible(calendar->widget.backend_widget, visible != 0) != 0) {
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

    visible = tinyui_calendar_get_header_visible((void *)calendar->widget.backend_widget);
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

    if (tinyui_calendar_set_header_format(calendar->widget.backend_widget, format) != 0) {
        return -1;
    }
    calendar->header_format = tinyui_calendar_get_header_format(calendar->widget.backend_widget);
    return calendar->header_format != 0 ? 0 : -1;
}

int picoui_calendar_set_bg_color(struct picoui_calendar *calendar, unsigned int rgb)
{
    if (calendar == 0 || rgb > 0xFFFFFFU) {
        return -1;
    }

    return tinyui_calendar_set_bg_color(calendar->widget.backend_widget, rgb);
}

int picoui_calendar_set_item_color(struct picoui_calendar *calendar, unsigned int rgb)
{
    if (calendar == 0 || rgb > 0xFFFFFFU) {
        return -1;
    }

    return tinyui_calendar_set_item_color(calendar->widget.backend_widget, rgb);
}

int picoui_calendar_set_text_color(struct picoui_calendar *calendar, unsigned int rgb)
{
    if (calendar == 0 || rgb > 0xFFFFFFU) {
        return -1;
    }

    return tinyui_calendar_set_text_color(calendar->widget.backend_widget, rgb);
}

const char *picoui_calendar_get_header_format(const struct picoui_calendar *calendar)
{
    const char *format;

    if (calendar == 0) {
        return 0;
    }

    format = tinyui_calendar_get_header_format((void *)calendar->widget.backend_widget);
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

    day = tinyui_calendar_get_grid_value((void *)calendar->widget.backend_widget, week, weekday);
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

    current = tinyui_calendar_is_current_month_cell((void *)calendar->widget.backend_widget,
                                                            week,
                                                            weekday);
    if (current >= 0) {
        index = week * 7 + weekday;
        ((struct picoui_calendar *)calendar)->grid_flags[index] = (unsigned char)current;
    }
    return current;
}
