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

#include "backend.h"
#include "internal.h"
#include "ldCalendar.h"

#include <stdlib.h>

extern const arm_2d_a1_font_t ARM_2D_FONT_6x8;

static uint8_t g_picoui_calendar_day_name_0[] = "Su";
static uint8_t g_picoui_calendar_day_name_1[] = "Mo";
static uint8_t g_picoui_calendar_day_name_2[] = "Tu";
static uint8_t g_picoui_calendar_day_name_3[] = "We";
static uint8_t g_picoui_calendar_day_name_4[] = "Th";
static uint8_t g_picoui_calendar_day_name_5[] = "Fr";
static uint8_t g_picoui_calendar_day_name_6[] = "Sa";
static uint8_t *g_picoui_calendar_day_names[7] = {
    g_picoui_calendar_day_name_0,
    g_picoui_calendar_day_name_1,
    g_picoui_calendar_day_name_2,
    g_picoui_calendar_day_name_3,
    g_picoui_calendar_day_name_4,
    g_picoui_calendar_day_name_5,
    g_picoui_calendar_day_name_6,
};

static ldColor picoui_backend_calendar_rgb_to_ld(unsigned int rgb)
{
    return (ldColor)rgb;
}

static struct picoui_backend_app_state *picoui_backend_calendar_get_app_state(void *parent)
{
    struct picoui_backend_widget *parent_widget = parent;

    if (parent_widget == NULL || parent_widget->owner == NULL || parent_widget->owner->backend_app == NULL) {
        return NULL;
    }
    return (struct picoui_backend_app_state *)parent_widget->owner->backend_app;
}

static ldCalendar_t *picoui_backend_calendar_get_ld(void *backend_widget)
{
    struct picoui_backend_widget *widget = backend_widget;

    if (widget == NULL || widget->ld_widget == NULL) {
        return NULL;
    }

    return (ldCalendar_t *)widget->ld_widget;
}

static int picoui_backend_calendar_sync_host_cache(struct picoui_backend_widget *backend)
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

    ld_calendar = picoui_backend_calendar_get_ld(backend);
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

/**
 * @brief Create backend for calendar
 *
 * @param[in] parent Parent widget
 * @param[in] id Widget identifier string
 */

void *picoui_backend_create_calendar(void *parent, const char *id)
{
    struct picoui_backend_widget *widget;
    struct picoui_backend_widget *parent_widget = parent;
    struct picoui_backend_app_state *app_state;
    ldCalendar_t *ld_calendar;
    uint16_t name_id;

    if (parent == 0 || id == 0) {
        return 0;
    }

    app_state = picoui_backend_calendar_get_app_state(parent);
    if (app_state == NULL || app_state->ld_scene == NULL || parent_widget->ld_widget == NULL) {
        return 0;
    }

    widget = calloc(1, sizeof(*widget));
    if (widget == 0) {
        return 0;
    }

    name_id = ++app_state->next_ld_name_id;
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

    ldCalendarSetDayNames(ld_calendar, g_picoui_calendar_day_names);
    ldCalendarSetHeader(ld_calendar, true);
    ldCalendarSetHeaderFormat(ld_calendar, (uint8_t *)"yyyy-mm-dd");
    widget->parent = parent;
    widget->id = id;
    widget->kind = PICOUI_BACKEND_WIDGET_CALENDAR;
    widget->theme = parent_widget->theme;
    widget->ld_widget = ld_calendar;
    widget->ld_name_id = name_id;
    if (picoui_backend_widget_attach_child(parent, widget) != 0) {
        free(widget);
        return 0;
    }
    return widget;
}

/**
 * @brief Set day names of calendar backend
 *
 * @param[in] backend_widget backend widget
 * @param[in] day_names[7 day names[7
 * @return 0 on success, -1 on failure
 */

int picoui_backend_calendar_set_day_names(void *backend_widget, const char *const day_names[7])
{
    ldCalendar_t *ld_calendar = picoui_backend_calendar_get_ld(backend_widget);
    int i;

    if (ld_calendar == NULL || day_names == NULL) {
        return -1;
    }

    for (i = 0; i < 7; ++i) {
        if (day_names[i] == NULL) {
            return -1;
        }
        g_picoui_calendar_day_names[i] = (uint8_t *)day_names[i];
    }

    ldCalendarSetDayNames(ld_calendar, g_picoui_calendar_day_names);
    return 0;
}

/**
 * @brief Set date of calendar backend
 *
 * @param[in] backend_widget backend widget
 * @param[in] year year
 * @param[in] month month
 * @param[in] day day
 * @return -1 on failure
 */

int picoui_backend_calendar_set_date(void *backend_widget, int year, int month, int day)
{
    struct picoui_backend_widget *backend = backend_widget;
    ldCalendar_t *ld_calendar = picoui_backend_calendar_get_ld(backend_widget);

    if (backend == NULL || ld_calendar == NULL || month < 1 || month > 12 || day < 1 || day > 31) {
        return -1;
    }

    ldCalendarSetDate(ld_calendar, (uint16_t)year, (uint8_t)month, (uint8_t)day);
    return picoui_backend_calendar_sync_host_cache(backend);
}

/**
 * @brief Get date from calendar backend
 *
 * @param[in] backend_widget backend widget
 * @param[in] year year
 * @param[in] month month
 * @param[in] day day
 * @return 0 on success, -1 on failure
 */

int picoui_backend_calendar_get_date(void *backend_widget, int *year, int *month, int *day)
{
    struct picoui_backend_widget *backend = backend_widget;
    struct picoui_calendar *calendar;

    if (backend == NULL || picoui_backend_calendar_sync_host_cache(backend) != 0) {
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

/**
 * @brief Set header visible of calendar backend
 *
 * @param[in] backend_widget backend widget
 * @param[in] visible Visibility state
 * @return -1 on failure
 */

int picoui_backend_calendar_set_header_visible(void *backend_widget, int visible)
{
    struct picoui_backend_widget *backend = backend_widget;
    ldCalendar_t *ld_calendar = picoui_backend_calendar_get_ld(backend_widget);

    if (backend == NULL || ld_calendar == NULL) {
        return -1;
    }

    ldCalendarSetHeader(ld_calendar, visible != 0);
    return picoui_backend_calendar_sync_host_cache(backend);
}

/**
 * @brief Get header visible from calendar backend
 *
 * @param[in] backend_widget backend widget
 * @return -1 on failure
 */

int picoui_backend_calendar_get_header_visible(void *backend_widget)
{
    struct picoui_backend_widget *backend = backend_widget;
    struct picoui_calendar *calendar;

    if (backend == NULL || picoui_backend_calendar_sync_host_cache(backend) != 0) {
        return -1;
    }

    calendar = (struct picoui_calendar *)backend->host_widget;
    return calendar->show_header;
}

/**
 * @brief Set header format of calendar backend
 *
 * @param[in] backend_widget backend widget
 * @param[in] format Format string
 * @return -1 on failure
 */

int picoui_backend_calendar_set_header_format(void *backend_widget, const char *format)
{
    struct picoui_backend_widget *backend = backend_widget;
    ldCalendar_t *ld_calendar = picoui_backend_calendar_get_ld(backend_widget);

    if (backend == NULL || ld_calendar == NULL || format == NULL) {
        return -1;
    }

    ldCalendarSetHeaderFormat(ld_calendar, (uint8_t *)format);
    return picoui_backend_calendar_sync_host_cache(backend);
}

/**
 * @brief Set bg color of calendar backend
 *
 * @param[in] backend_widget backend widget
 * @param[in] rgb RGB color value (0xRRGGBB)
 * @return 0 on success, -1 on failure
 */

int picoui_backend_calendar_set_bg_color(void *backend_widget, unsigned int rgb)
{
    ldCalendar_t *ld_calendar = picoui_backend_calendar_get_ld(backend_widget);

    if (ld_calendar == NULL || rgb > 0xFFFFFFU) {
        return -1;
    }

    ld_calendar->bgColor = picoui_backend_calendar_rgb_to_ld(rgb);
    return 0;
}

/**
 * @brief Set item color of calendar backend
 *
 * @param[in] backend_widget backend widget
 * @param[in] rgb RGB color value (0xRRGGBB)
 * @return 0 on success, -1 on failure
 */

int picoui_backend_calendar_set_item_color(void *backend_widget, unsigned int rgb)
{
    ldCalendar_t *ld_calendar = picoui_backend_calendar_get_ld(backend_widget);

    if (ld_calendar == NULL || rgb > 0xFFFFFFU) {
        return -1;
    }

    ld_calendar->itemColor = picoui_backend_calendar_rgb_to_ld(rgb);
    return 0;
}

/**
 * @brief Set text color of calendar backend
 *
 * @param[in] backend_widget backend widget
 * @param[in] rgb RGB color value (0xRRGGBB)
 * @return 0 on success, -1 on failure
 */

int picoui_backend_calendar_set_text_color(void *backend_widget, unsigned int rgb)
{
    ldCalendar_t *ld_calendar = picoui_backend_calendar_get_ld(backend_widget);

    if (ld_calendar == NULL || rgb > 0xFFFFFFU) {
        return -1;
    }

    ld_calendar->textColor = picoui_backend_calendar_rgb_to_ld(rgb);
    return 0;
}

/**
 * @brief Set use system date of calendar backend
 *
 * @param[in] backend_widget backend widget
 * @param[in] enabled Enable state
 * @return -1 on failure
 */

int picoui_backend_calendar_set_use_system_date(void *backend_widget, int enabled)
{
    struct picoui_backend_widget *backend = backend_widget;
    ldCalendar_t *ld_calendar = picoui_backend_calendar_get_ld(backend_widget);

    if (backend == NULL || ld_calendar == NULL) {
        return -1;
    }

    ldCalendarSetAutoSysDate(ld_calendar, enabled != 0);
    if (enabled != 0) {
        ldCalendar_on_frame_start(NULL, ld_calendar);
    }
    return picoui_backend_calendar_sync_host_cache(backend);
}

/**
 * @brief Get use system date from calendar backend
 *
 * @param[in] backend_widget backend widget
 * @param[in] enabled Enable state
 * @return 0 on success, -1 on failure
 */

int picoui_backend_calendar_get_use_system_date(void *backend_widget, int *enabled)
{
    ldCalendar_t *ld_calendar = picoui_backend_calendar_get_ld(backend_widget);

    if (ld_calendar == NULL || enabled == NULL) {
        return -1;
    }

    *enabled = ld_calendar->isAutoSysDate ? 1 : 0;
    return 0;
}

/**
 * @brief Get header format from calendar backend
 *
 * @param[in] backend_widget backend widget
 */

const char *picoui_backend_calendar_get_header_format(void *backend_widget)
{
    struct picoui_backend_widget *backend = backend_widget;
    struct picoui_calendar *calendar;

    if (backend == NULL || picoui_backend_calendar_sync_host_cache(backend) != 0) {
        return NULL;
    }

    calendar = (struct picoui_calendar *)backend->host_widget;
    return calendar->header_format;
}

/**
 * @brief Get grid value from calendar backend
 *
 * @param[in] backend_widget backend widget
 * @param[in] week week
 * @param[in] weekday weekday
 * @return -1 on failure
 */

int picoui_backend_calendar_get_grid_value(void *backend_widget, int week, int weekday)
{
    struct picoui_backend_widget *backend = backend_widget;
    struct picoui_calendar *calendar;
    int index;

    if (backend == NULL || week < 0 || week >= 6 || weekday < 0 || weekday >= 7 ||
        picoui_backend_calendar_sync_host_cache(backend) != 0) {
        return -1;
    }

    calendar = (struct picoui_calendar *)backend->host_widget;
    index = week * 7 + weekday;
    return (int)calendar->grid_values[index];
}

/**
 * @brief Check is current month cell of calendar
 *
 * @param[in] backend_widget backend widget
 * @param[in] week week
 * @param[in] weekday weekday
 * @return -1 on failure
 */

int picoui_backend_calendar_is_current_month_cell(void *backend_widget, int week, int weekday)
{
    struct picoui_backend_widget *backend = backend_widget;
    struct picoui_calendar *calendar;
    int index;

    if (backend == NULL || week < 0 || week >= 6 || weekday < 0 || weekday >= 7 ||
        picoui_backend_calendar_sync_host_cache(backend) != 0) {
        return -1;
    }

    calendar = (struct picoui_calendar *)backend->host_widget;
    index = week * 7 + weekday;
    return calendar->grid_flags[index] != 0 ? 1 : 0;
}
