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

#include "../backend/ldgui/backend.h"
#include "internal.h"
#include "picoui/calendar.h"

#include <stdlib.h>

#define PICOUI_CALENDAR_STATE_MAX 32

struct picoui_calendar_state_entry {
    const struct picoui_calendar *calendar;
    picoui_calendar_selected_callback_t cb;
    void *user_data;
    const char *day_names[7];
};

static struct picoui_calendar_state_entry g_picoui_calendar_states[PICOUI_CALENDAR_STATE_MAX];
static const char *g_picoui_calendar_default_day_names[7] = {
    "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat",
};

static int picoui_calendar_is_leap_year(int year)
{
    if ((year % 400) == 0) {
        return 1;
    }
    if ((year % 100) == 0) {
        return 0;
    }
    return (year % 4) == 0;
}

static int picoui_calendar_days_in_month(int year, int month)
{
    static const int days[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

    if (month < 1 || month > 12) {
        return 0;
    }
    if (month == 2 && picoui_calendar_is_leap_year(year)) {
        return 29;
    }
    return days[month - 1];
}

static int picoui_calendar_day_of_week(int year, int month, int day)
{
    static const int month_offsets[] = {0, 3, 2, 5, 0, 3, 5, 1, 4, 6, 2, 4};

    if (month < 3) {
        year -= 1;
    }
    return (year + year / 4 - year / 100 + year / 400 + month_offsets[month - 1] + day) % 7;
}

static int picoui_calendar_props_are_valid(const struct picoui_calendar_props *props)
{
    return props != 0
        && props->id != 0
        && props->month >= 1
        && props->month <= 12
        && props->day >= 1
        && props->day <= 31
        && props->width >= 0
        && props->height >= 0
        && props->header_format != 0;
}

static struct picoui_calendar_state_entry *picoui_calendar_find_state(
    const struct picoui_calendar *calendar)
{
    int i;

    if (calendar == 0) {
        return 0;
    }

    for (i = 0; i < PICOUI_CALENDAR_STATE_MAX; ++i) {
        if (g_picoui_calendar_states[i].calendar == calendar) {
            return &g_picoui_calendar_states[i];
        }
    }

    return 0;
}

static struct picoui_calendar_state_entry *picoui_calendar_alloc_state(
    const struct picoui_calendar *calendar)
{
    struct picoui_calendar_state_entry *state;
    int i;

    state = picoui_calendar_find_state(calendar);
    if (state != 0) {
        return state;
    }

    if (calendar == 0) {
        return 0;
    }

    for (i = 0; i < PICOUI_CALENDAR_STATE_MAX; ++i) {
        if (g_picoui_calendar_states[i].calendar == 0) {
            g_picoui_calendar_states[i].calendar = calendar;
            g_picoui_calendar_states[i].cb = 0;
            g_picoui_calendar_states[i].user_data = 0;
            g_picoui_calendar_states[i].day_names[0] = g_picoui_calendar_default_day_names[0];
            g_picoui_calendar_states[i].day_names[1] = g_picoui_calendar_default_day_names[1];
            g_picoui_calendar_states[i].day_names[2] = g_picoui_calendar_default_day_names[2];
            g_picoui_calendar_states[i].day_names[3] = g_picoui_calendar_default_day_names[3];
            g_picoui_calendar_states[i].day_names[4] = g_picoui_calendar_default_day_names[4];
            g_picoui_calendar_states[i].day_names[5] = g_picoui_calendar_default_day_names[5];
            g_picoui_calendar_states[i].day_names[6] = g_picoui_calendar_default_day_names[6];
            return &g_picoui_calendar_states[i];
        }
    }

    return 0;
}

static int picoui_calendar_sync_grid(struct picoui_calendar *calendar)
{
    int first_weekday;
    int days_in_month;
    int prev_month;
    int prev_year;
    int prev_days;
    int row;
    int col;
    int index;
    int day;

    if (calendar == 0) {
        return -1;
    }

    if (calendar->month < 1 || calendar->month > 12) {
        return -1;
    }

    days_in_month = picoui_calendar_days_in_month(calendar->year, calendar->month);
    if (calendar->day < 1 || calendar->day > days_in_month) {
        return -1;
    }

    first_weekday = picoui_calendar_day_of_week(calendar->year, calendar->month, 1);
    prev_month = calendar->month - 1;
    prev_year = calendar->year;
    if (prev_month < 1) {
        prev_month = 12;
        prev_year--;
    }
    prev_days = picoui_calendar_days_in_month(prev_year, prev_month);
    day = 1;

    for (row = 0; row < 6; ++row) {
        for (col = 0; col < 7; ++col) {
            index = row * 7 + col;
            if (index < first_weekday) {
                calendar->grid_values[index] =
                    (unsigned char)(prev_days - (first_weekday - index) + 1);
                calendar->grid_flags[index] = 0;
            } else if (day <= days_in_month) {
                calendar->grid_values[index] = (unsigned char)day;
                calendar->grid_flags[index] = 1;
                day++;
            } else {
                calendar->grid_values[index] = (unsigned char)(day - days_in_month);
                calendar->grid_flags[index] = 0;
                day++;
            }
        }
    }

    return 0;
}

struct picoui_calendar *picoui_calendar_create(struct picoui_window *parent, const char *id)
{
    struct picoui_calendar *calendar;
    struct picoui_calendar_state_entry *state;
    struct picoui_backend_widget *backend;

    if (parent == 0 || id == 0 || parent->widget.backend_widget == 0) {
        return 0;
    }

    calendar = calloc(1, sizeof(*calendar));
    if (calendar == 0) {
        return 0;
    }

    calendar->widget.backend_widget = picoui_backend_create_calendar(parent->widget.backend_widget, id);
    if (calendar->widget.backend_widget == 0) {
        free(calendar);
        return 0;
    }

    calendar->id = id;
    calendar->widget.visible = 1;
    calendar->widget.enabled = 1;
    calendar->header_format = "yyyy-mm-dd";
    calendar->show_header = 1;

    if (picoui_backend_widget_bind_host(calendar->widget.backend_widget, &calendar->widget) != 0) {
        free(calendar);
        return 0;
    }

    state = picoui_calendar_alloc_state(calendar);
    backend = (struct picoui_backend_widget *)calendar->widget.backend_widget;
    if (state == 0 || backend == 0) {
        free(calendar);
        return 0;
    }

    if (picoui_calendar_set_date(calendar, 2026, 6, 15) != 0
        || picoui_calendar_set_selected_date(calendar, 2026, 6, 15) != 0) {
        free(calendar);
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
    if (props->style_class != 0
        && picoui_widget_set_style_class(&calendar->widget, props->style_class) != 0) {
        free(calendar);
        return 0;
    }
    if ((props->width > 0 || props->height > 0)
        && picoui_widget_set_size(&calendar->widget, props->width, props->height) != 0) {
        free(calendar);
        return 0;
    }
    if (picoui_calendar_set_date(calendar, props->year, props->month, props->day) != 0
        || picoui_calendar_set_selected_date(calendar, props->year, props->month, props->day) != 0
        || picoui_calendar_set_header_visible(calendar, props->show_header) != 0
        || picoui_calendar_set_header_format(calendar, props->header_format) != 0) {
        free(calendar);
        return 0;
    }

    return calendar;
}

int picoui_calendar_set_date(struct picoui_calendar *calendar, int year, int month, int day)
{
    int days_in_month;
    struct picoui_backend_widget *backend;

    if (calendar == 0 || month < 1 || month > 12) {
        return -1;
    }

    days_in_month = picoui_calendar_days_in_month(year, month);
    if (day < 1 || day > days_in_month) {
        return -1;
    }

    backend = (struct picoui_backend_widget *)calendar->widget.backend_widget;
    if (backend != 0 && backend->ld_widget != 0
        && picoui_backend_calendar_set_date(backend, year, month, day) != 0) {
        return -1;
    }
    calendar->year = year;
    calendar->month = month;
    calendar->day = day;
    return picoui_calendar_sync_grid(calendar);
}

int picoui_calendar_get_date(const struct picoui_calendar *calendar, int *year, int *month, int *day)
{
    struct picoui_backend_widget *backend;

    if (calendar == 0) {
        return -1;
    }

    backend = (struct picoui_backend_widget *)calendar->widget.backend_widget;
    if (((struct picoui_calendar *)calendar)->use_system_date
        && backend != 0
        && backend->ld_widget != 0
        && picoui_backend_calendar_get_date(backend, &((struct picoui_calendar *)calendar)->year,
                                            &((struct picoui_calendar *)calendar)->month,
                                            &((struct picoui_calendar *)calendar)->day) == 0) {
        (void)picoui_calendar_sync_grid((struct picoui_calendar *)calendar);
    }

    if (year != 0) {
        *year = calendar->year;
    }
    if (month != 0) {
        *month = calendar->month;
    }
    if (day != 0) {
        *day = calendar->day;
    }
    return 0;
}

int picoui_calendar_set_selected_date(struct picoui_calendar *calendar, int year, int month, int day)
{
    struct picoui_backend_widget *backend;

    if (calendar == 0 || month < 1 || month > 12) {
        return -1;
    }

    backend = (struct picoui_backend_widget *)calendar->widget.backend_widget;
    if (backend == 0) {
        return -1;
    }

    if (day < 1 || day > picoui_calendar_days_in_month(year, month)) {
        return -1;
    }

    if (backend->ld_widget != 0 && picoui_backend_calendar_set_selected_date(backend, year, month, day) != 0) {
        return -1;
    }

    calendar->year = year;
    calendar->month = month;
    calendar->day = day;
    if (picoui_calendar_sync_grid(calendar) != 0) {
        return -1;
    }
    backend->value = (year << 16) | (month << 8) | day;
    backend->data_model_epoch++;
    backend->last_data_source = PICOUI_BACKEND_DATA_SOURCE_SETTER;
    return 0;
}

int picoui_calendar_get_selected_date(const struct picoui_calendar *calendar,
                                      int *year,
                                      int *month,
                                      int *day)
{
    struct picoui_backend_widget *backend;
    int selected_year = 0;
    int selected_month = 0;
    int selected_day = 0;

    if (calendar == 0) {
        return -1;
    }

    backend = (struct picoui_backend_widget *)calendar->widget.backend_widget;
    if (backend == 0) {
        return -1;
    }
    if (backend->ld_widget != 0
        && picoui_backend_calendar_get_selected_date(backend,
                                                     &selected_year,
                                                     &selected_month,
                                                     &selected_day) != 0) {
        return -1;
    }
    if (backend->ld_widget == 0) {
        selected_year = calendar->year;
        selected_month = calendar->month;
        selected_day = calendar->day;
    }

    if (year != 0) {
        *year = selected_year;
    }
    if (month != 0) {
        *month = selected_month;
    }
    if (day != 0) {
        *day = selected_day;
    }
    return 0;
}

void picoui_calendar_set_on_selected(struct picoui_calendar *calendar,
                                     picoui_calendar_selected_callback_t callback,
                                     void *user_data)
{
    struct picoui_calendar_state_entry *state;

    state = picoui_calendar_alloc_state(calendar);
    if (state == 0) {
        return;
    }

    state->cb = callback;
    state->user_data = user_data;
    calendar->cb = callback;
    calendar->user_data = user_data;
}

int picoui_calendar_set_use_system_date(struct picoui_calendar *calendar, int enabled)
{
    struct picoui_backend_widget *backend;

    if (calendar == 0) {
        return -1;
    }

    calendar->use_system_date = enabled != 0;
    backend = (struct picoui_backend_widget *)calendar->widget.backend_widget;
    if (backend != 0 && backend->ld_widget != 0
        && picoui_backend_calendar_set_use_system_date(backend, enabled) != 0) {
        return -1;
    }
    return 0;
}

int picoui_calendar_get_use_system_date(const struct picoui_calendar *calendar)
{
    int enabled;
    struct picoui_backend_widget *backend;

    if (calendar == 0) {
        return -1;
    }

    backend = (struct picoui_backend_widget *)calendar->widget.backend_widget;
    if (backend != 0 && backend->ld_widget != 0
        && picoui_backend_calendar_get_use_system_date(backend, &enabled) == 0) {
        ((struct picoui_calendar *)calendar)->use_system_date = enabled;
    }

    return calendar->use_system_date;
}

int picoui_calendar_set_day_names(struct picoui_calendar *calendar, const char *const day_names[7])
{
    struct picoui_calendar_state_entry *state;
    struct picoui_backend_widget *backend;
    int i;

    if (calendar == 0 || day_names == 0) {
        return -1;
    }

    state = picoui_calendar_alloc_state(calendar);
    if (state == 0) {
        return -1;
    }

    for (i = 0; i < 7; ++i) {
        if (day_names[i] == 0) {
            return -1;
        }
        state->day_names[i] = day_names[i];
    }
    backend = (struct picoui_backend_widget *)calendar->widget.backend_widget;
    if (backend != 0 && backend->ld_widget != 0
        && picoui_backend_calendar_set_day_names(backend, day_names) != 0) {
        return -1;
    }
    return 0;
}

int picoui_calendar_set_header_visible(struct picoui_calendar *calendar, int visible)
{
    struct picoui_backend_widget *backend;

    if (calendar == 0) {
        return -1;
    }

    calendar->show_header = visible != 0;
    backend = (struct picoui_backend_widget *)calendar->widget.backend_widget;
    if (backend != 0 && backend->ld_widget != 0
        && picoui_backend_calendar_set_header_visible(backend, visible) != 0) {
        return -1;
    }
    return 0;
}

int picoui_calendar_get_header_visible(const struct picoui_calendar *calendar)
{
    int visible;
    struct picoui_backend_widget *backend;

    if (calendar == 0) {
        return -1;
    }

    backend = (struct picoui_backend_widget *)calendar->widget.backend_widget;
    if (backend != 0 && backend->ld_widget != 0) {
        visible = picoui_backend_calendar_get_header_visible(backend);
        if (visible >= 0) {
            ((struct picoui_calendar *)calendar)->show_header = visible;
        }
    }

    return calendar->show_header;
}

int picoui_calendar_set_header_format(struct picoui_calendar *calendar, const char *format)
{
    struct picoui_backend_widget *backend;

    if (calendar == 0 || format == 0) {
        return -1;
    }

    calendar->header_format = format;
    backend = (struct picoui_backend_widget *)calendar->widget.backend_widget;
    if (backend != 0 && backend->ld_widget != 0
        && picoui_backend_calendar_set_header_format(backend, format) != 0) {
        return -1;
    }
    return 0;
}

int picoui_calendar_set_bg_color(struct picoui_calendar *calendar, unsigned int rgb)
{
    struct picoui_backend_widget *backend;

    if (calendar == 0 || rgb > 0xFFFFFFU) {
        return -1;
    }

    if (picoui_widget_set_bg_color(&calendar->widget, rgb) != 0) {
        return -1;
    }
    backend = (struct picoui_backend_widget *)calendar->widget.backend_widget;
    if (backend != 0 && backend->ld_widget != 0) {
        return picoui_backend_calendar_set_bg_color(backend, rgb);
    }
    return 0;
}

int picoui_calendar_set_item_color(struct picoui_calendar *calendar, unsigned int rgb)
{
    struct picoui_backend_widget *backend;

    if (calendar == 0 || rgb > 0xFFFFFFU) {
        return -1;
    }

    if (picoui_widget_set_border_color(&calendar->widget, rgb) != 0) {
        return -1;
    }
    backend = (struct picoui_backend_widget *)calendar->widget.backend_widget;
    if (backend != 0 && backend->ld_widget != 0) {
        return picoui_backend_calendar_set_item_color(backend, rgb);
    }
    return 0;
}

int picoui_calendar_set_text_color(struct picoui_calendar *calendar, unsigned int rgb)
{
    struct picoui_backend_widget *backend;

    if (calendar == 0 || rgb > 0xFFFFFFU) {
        return -1;
    }

    if (picoui_widget_set_text_color(&calendar->widget, rgb) != 0) {
        return -1;
    }
    backend = (struct picoui_backend_widget *)calendar->widget.backend_widget;
    if (backend != 0 && backend->ld_widget != 0) {
        return picoui_backend_calendar_set_text_color(backend, rgb);
    }
    return 0;
}

const char *picoui_calendar_get_header_format(const struct picoui_calendar *calendar)
{
    const char *format;
    struct picoui_backend_widget *backend;

    if (calendar == 0) {
        return 0;
    }

    backend = (struct picoui_backend_widget *)calendar->widget.backend_widget;
    if (backend != 0 && backend->ld_widget != 0) {
        format = picoui_backend_calendar_get_header_format(backend);
        if (format != 0) {
            ((struct picoui_calendar *)calendar)->header_format = format;
        }
    }

    return calendar->header_format;
}

int picoui_calendar_get_grid_value(const struct picoui_calendar *calendar, int week, int weekday)
{
    int index;

    if (calendar == 0 || week < 0 || week >= 6 || weekday < 0 || weekday >= 7) {
        return -1;
    }

    index = week * 7 + weekday;
    return calendar->grid_values[index];
}

int picoui_calendar_is_current_month_cell(const struct picoui_calendar *calendar, int week, int weekday)
{
    int index;

    if (calendar == 0 || week < 0 || week >= 6 || weekday < 0 || weekday >= 7) {
        return -1;
    }

    index = week * 7 + weekday;
    return calendar->grid_flags[index];
}
