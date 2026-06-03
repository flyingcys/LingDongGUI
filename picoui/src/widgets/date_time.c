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
#include "picoui/date_time.h"
#include "picoui/widget.h"

#include <stdlib.h>

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

/**
 * @brief Create date time widget
 *
 * @param[in] parent Parent widget
 * @param[in] id Widget identifier string
 * @return Pointer to the object on success, NULL on failure
 */

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
        || picoui_date_time_set_text_color(dt, 0x000000U) != 0
        || picoui_date_time_set_bg_color(dt, 0xFFFFFFU) != 0
        || picoui_date_time_set_align(dt, PICOUI_ALIGN_CENTER) != 0
        || picoui_date_time_set_transparent(dt, 0) != 0
        || picoui_date_time_set_date(dt, 2026, 1, 1) != 0
        || picoui_date_time_set_time(dt, 12, 0, 0) != 0) {
        free(dt);
        return 0;
    }
    return dt;
}

/**
 * @brief date time init
 *
 * @param[in] parent Parent widget
 * @param[in] id Widget identifier string
 * @return Pointer to the object
 */

struct picoui_date_time *picoui_date_time_init(struct picoui_widget *parent, const char *id)
{
    return picoui_date_time_create(parent, id);
}

/**
 * @brief Create date time widget with properties
 *
 * @param[in] parent Parent widget
 * @param[in] props Properties structure
 * @return Pointer to the object on success, NULL on failure
 */

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
        || picoui_date_time_set_text_color(dt, props->text_color) != 0
        || picoui_date_time_set_bg_color(dt, props->bg_color) != 0
        || picoui_date_time_set_align(dt, props->align) != 0
        || picoui_date_time_set_transparent(dt, props->transparent) != 0
        || picoui_date_time_set_date(dt, props->year, props->month, props->day) != 0
        || picoui_date_time_set_time(dt, props->hour, props->minute, props->second) != 0) {
        free(dt);
        return 0;
    }

    return dt;
}

/**
 * @brief Set format of date time widget
 *
 * @param[in] dt dt
 * @param[in] format Format string
 * @return 0 on success, -1 on failure
 */

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

/**
 * @brief Set date of date time widget
 *
 * @param[in] dt dt
 * @param[in] year year
 * @param[in] month month
 * @param[in] day day
 * @return 0 on success, -1 on failure
 */

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

/**
 * @brief Set time of date time widget
 *
 * @param[in] dt dt
 * @param[in] hour hour
 * @param[in] minute minute
 * @param[in] second second
 * @return 0 on success, -1 on failure
 */

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

/**
 * @brief Set text color of date time widget
 *
 * @param[in] dt dt
 * @param[in] rgb RGB color value (0xRRGGBB)
 * @return 0 on success, -1 on failure
 */

int picoui_date_time_set_text_color(struct picoui_date_time *dt, unsigned int rgb)
{
    if (dt == 0 || rgb > 0xFFFFFFU) {
        return -1;
    }

    if (picoui_backend_date_time_set_text_color(dt, rgb) != 0) {
        return -1;
    }

    dt->text_color = rgb;
    return 0;
}

/**
 * @brief Set bg color of date time widget
 *
 * @param[in] dt dt
 * @param[in] rgb RGB color value (0xRRGGBB)
 * @return 0 on success, -1 on failure
 */

int picoui_date_time_set_bg_color(struct picoui_date_time *dt, unsigned int rgb)
{
    if (dt == 0 || rgb > 0xFFFFFFU) {
        return -1;
    }

    if (picoui_backend_date_time_set_bg_color(dt, rgb) != 0) {
        return -1;
    }

    dt->bg_color = rgb;
    return 0;
}

/**
 * @brief Set background color of date time widget
 *
 * @param[in] dt dt
 * @param[in] rgb RGB color value (0xRRGGBB)
 * @return 0 on success, -1 on failure
 */

int picoui_date_time_set_background_color(struct picoui_date_time *dt, unsigned int rgb)
{
    return picoui_date_time_set_bg_color(dt, rgb);
}

/**
 * @brief Set align of date time widget
 *
 * @param[in] dt dt
 * @param[in] align align
 * @return 0 on success, -1 on failure
 */

int picoui_date_time_set_align(struct picoui_date_time *dt, enum picoui_align align)
{
    if (dt == 0
        || (align != PICOUI_ALIGN_START
            && align != PICOUI_ALIGN_CENTER
            && align != PICOUI_ALIGN_END)) {
        return -1;
    }

    if (picoui_backend_date_time_set_align(dt, align) != 0) {
        return -1;
    }

    dt->align = align;
    return 0;
}

/**
 * @brief Set transparent of date time widget
 *
 * @param[in] dt dt
 * @param[in] transparent transparent
 * @return 0 on success, -1 on failure
 */

int picoui_date_time_set_transparent(struct picoui_date_time *dt, int transparent)
{
    if (dt == 0) {
        return -1;
    }

    if (picoui_backend_date_time_set_transparent(dt, transparent) != 0) {
        return -1;
    }

    dt->transparent = transparent != 0;
    return 0;
}

/**
 * @brief Set use system time of date time widget
 *
 * @param[in] dt dt
 * @param[in] enabled Enable state
 * @return 0 on success, -1 on failure
 */

int picoui_date_time_set_use_system_time(struct picoui_date_time *dt, int enabled)
{
    if (dt == 0) {
        return -1;
    }

    if (picoui_backend_date_time_set_use_system_time(dt, enabled) != 0) {
        return -1;
    }

    dt->use_system_time = enabled != 0;
    return 0;
}

/**
 * @brief Get format of date time widget
 *
 * @param[in] dt dt
 */

const char *picoui_date_time_get_format(const struct picoui_date_time *dt)
{
    if (dt == 0) {
        return 0;
    }

    return picoui_backend_date_time_get_format((struct picoui_date_time *)dt);
}

/**
 * @brief Get date of date time widget
 *
 * @param[in] dt dt
 * @param[in] year year
 * @param[in] month month
 * @param[in] day day
 * @return 0 on success, -1 on failure
 */

int picoui_date_time_get_date(const struct picoui_date_time *dt, int *year, int *month, int *day)
{
    if (dt == 0 || year == 0 || month == 0 || day == 0) {
        return -1;
    }

    *year = dt->year;
    *month = dt->month;
    *day = dt->day;
    return 0;
}

/**
 * @brief Get time of date time widget
 *
 * @param[in] dt dt
 * @param[in] hour hour
 * @param[in] minute minute
 * @param[in] second second
 * @return 0 on success, -1 on failure
 */

int picoui_date_time_get_time(const struct picoui_date_time *dt, int *hour, int *minute, int *second)
{
    if (dt == 0 || hour == 0 || minute == 0 || second == 0) {
        return -1;
    }

    *hour = dt->hour;
    *minute = dt->minute;
    *second = dt->second;
    return 0;
}

/**
 * @brief Get transparent of date time widget
 *
 * @param[in] dt dt
 * @return -1 on failure
 */

int picoui_date_time_get_transparent(const struct picoui_date_time *dt)
{
    if (dt == 0) {
        return -1;
    }

    return dt->transparent;
}

/**
 * @brief Get use system time of date time widget
 *
 * @param[in] dt dt
 * @return -1 on failure
 */

int picoui_date_time_get_use_system_time(const struct picoui_date_time *dt)
{
    int enabled = 0;

    if (dt == 0) {
        return -1;
    }

    if (picoui_backend_date_time_get_use_system_time((struct picoui_date_time *)dt, &enabled) != 0) {
        return -1;
    }

    ((struct picoui_date_time *)dt)->use_system_time = enabled;
    return enabled;
}
