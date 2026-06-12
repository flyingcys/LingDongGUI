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
#include "../core/runtime_bridge.h"
#include "../../../src/gui/ldDateTime.h"

#include <stdlib.h>

extern const arm_2d_a1_font_t ARM_2D_FONT_6x8;

static ldColor picoui_date_time_rgb_to_ld_color(unsigned int rgb)
{
    return __RGB((rgb >> 16) & 0xFFU, (rgb >> 8) & 0xFFU, rgb & 0xFFU);
}

static int picoui_date_time_map_align(enum picoui_align align, arm_2d_align_t *out)
{
    if (out == NULL) {
        return -1;
    }

    switch (align) {
    case PICOUI_ALIGN_START:
        *out = ARM_2D_ALIGN_LEFT;
        return 0;
    case PICOUI_ALIGN_CENTER:
        *out = ARM_2D_ALIGN_CENTRE;
        return 0;
    case PICOUI_ALIGN_END:
        *out = ARM_2D_ALIGN_RIGHT;
        return 0;
    default:
        return -1;
    }
}

static ldDateTime_t *picoui_date_time_get_ld(struct picoui_date_time *dt)
{
    struct picoui_backend_widget *backend;

    if (dt == NULL || dt->widget.backend_widget == NULL) {
        return NULL;
    }

    backend = (struct picoui_backend_widget *)dt->widget.backend_widget;
    if (backend->kind != PICOUI_BACKEND_WIDGET_DATE_TIME || backend->ld_widget == NULL) {
        return NULL;
    }

    return (ldDateTime_t *)backend->ld_widget;
}

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
    struct picoui_backend_widget *backend;
    struct picoui_backend_widget *parent_backend;
    struct picoui_backend_app_state *app_state;
    ldDateTime_t *ld_date_time;
    uint16_t name_id;

    if (parent == 0 || id == 0 || parent->backend_widget == 0) {
        return 0;
    }

    parent_backend = (struct picoui_backend_widget *)parent->backend_widget;
    app_state = tinyui_runtime_bridge_backend_state_from_parent(parent_backend);
    if (parent_backend->ld_widget == 0 || app_state == 0 || app_state->ld_scene == 0) {
        return 0;
    }

    dt = calloc(1, sizeof(*dt));
    if (dt == 0) {
        return 0;
    }

    backend = calloc(1, sizeof(*backend));
    if (backend == 0) {
        free(dt);
        return 0;
    }

    name_id = tinyui_runtime_bridge_next_name_id(parent_backend);
    if (name_id == 0) {
        free(backend);
        free(dt);
        return 0;
    }

    ld_date_time = ldDateTime_init(app_state->ld_scene,
                                   NULL,
                                   name_id,
                                   parent_backend->ld_name_id,
                                   0,
                                   0,
                                   240,
                                   32,
                                   (arm_2d_font_t *)&ARM_2D_FONT_6x8);
    if (ld_date_time == 0) {
        free(backend);
        free(dt);
        return 0;
    }

    if (tinyui_widget_init_child(backend,
                                         parent_backend,
                                         PICOUI_BACKEND_WIDGET_DATE_TIME,
                                         id,
                                         parent_backend->theme) != 0) {
        ldDateTime_depose(app_state->ld_scene, ld_date_time);
        free(backend);
        free(dt);
        return 0;
    }
    backend->ld_widget = ld_date_time;
    backend->ld_name_id = name_id;
    backend->text = (const char *)ld_date_time->formatStr;
    if (tinyui_widget_attach_child(parent_backend, backend) != 0) {
        ldDateTime_depose(app_state->ld_scene, ld_date_time);
        free(backend);
        free(dt);
        return 0;
    }

    dt->widget.backend_widget = backend;
    dt->id = id;
    dt->widget.visible = 1;
    dt->widget.enabled = 1;
    if (tinyui_runtime_bridge_bind_host(dt->widget.backend_widget, &dt->widget) != 0) {
        (void)tinyui_runtime_bridge_detach_from_parent(dt->widget.backend_widget);
        ldDateTime_depose(app_state->ld_scene, ld_date_time);
        free(backend);
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
    ldDateTime_t *ld_date_time;
    struct picoui_backend_widget *backend;

    if (dt == 0 || format == 0) {
        return -1;
    }

    ld_date_time = picoui_date_time_get_ld(dt);
    if (ld_date_time == NULL) {
        return -1;
    }

    ldDateTimeSetFormat(ld_date_time, (const uint8_t *)format);
    backend = (struct picoui_backend_widget *)dt->widget.backend_widget;
    backend->text = format;
    dt->format = format;
    dt->use_system_time = 0;
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
    ldDateTime_t *ld_date_time;

    if (dt == 0 || month < 1 || month > 12 || day < 1 || day > 31) {
        return -1;
    }

    ld_date_time = picoui_date_time_get_ld(dt);
    if (ld_date_time == NULL) {
        return -1;
    }

    ldDateTimeSetDate(ld_date_time, (uint16_t)year, (uint8_t)month, (uint8_t)day);
    dt->year = year;
    dt->month = month;
    dt->day = day;
    dt->use_system_time = 0;
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
    ldDateTime_t *ld_date_time;

    if (dt == 0
        || hour < 0 || hour > 23
        || minute < 0 || minute > 59
        || second < 0 || second > 59) {
        return -1;
    }

    ld_date_time = picoui_date_time_get_ld(dt);
    if (ld_date_time == NULL) {
        return -1;
    }

    ldDateTimeSetTime(ld_date_time, (uint8_t)hour, (uint8_t)minute, (uint8_t)second);
    dt->hour = hour;
    dt->minute = minute;
    dt->second = second;
    dt->use_system_time = 0;
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
    ldDateTime_t *ld_date_time;

    if (dt == 0 || rgb > 0xFFFFFFU) {
        return -1;
    }

    ld_date_time = picoui_date_time_get_ld(dt);
    if (ld_date_time == NULL) {
        return -1;
    }

    ldDateTimeSetTextColor(ld_date_time, picoui_date_time_rgb_to_ld_color(rgb));
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
    ldDateTime_t *ld_date_time;

    if (dt == 0 || rgb > 0xFFFFFFU) {
        return -1;
    }

    ld_date_time = picoui_date_time_get_ld(dt);
    if (ld_date_time == NULL) {
        return -1;
    }

    ldDateTimeSetBackgroundColor(ld_date_time, picoui_date_time_rgb_to_ld_color(rgb));
    dt->bg_color = rgb;
    dt->transparent = 0;
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
    ldDateTime_t *ld_date_time;
    arm_2d_align_t native_align;

    if (dt == 0
        || (align != PICOUI_ALIGN_START
            && align != PICOUI_ALIGN_CENTER
            && align != PICOUI_ALIGN_END)) {
        return -1;
    }

    ld_date_time = picoui_date_time_get_ld(dt);
    if (ld_date_time == NULL || picoui_date_time_map_align(align, &native_align) != 0) {
        return -1;
    }

    ldDateTimeSetAlign(ld_date_time, native_align);
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
    ldDateTime_t *ld_date_time;

    if (dt == 0) {
        return -1;
    }

    ld_date_time = picoui_date_time_get_ld(dt);
    if (ld_date_time == NULL) {
        return -1;
    }

    ldDateTimeSetTransparent(ld_date_time, transparent != 0);
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
    ldDateTime_t *ld_date_time;

    if (dt == 0) {
        return -1;
    }

    ld_date_time = picoui_date_time_get_ld(dt);
    if (ld_date_time == NULL) {
        return -1;
    }

    ld_date_time->isAutoSysTime = enabled != 0;
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
    ldDateTime_t *ld_date_time;

    if (dt == 0) {
        return 0;
    }

    ld_date_time = picoui_date_time_get_ld((struct picoui_date_time *)dt);
    if (ld_date_time == NULL) {
        return 0;
    }

    ((struct picoui_date_time *)dt)->format = (const char *)ld_date_time->formatStr;
    return ((struct picoui_date_time *)dt)->format;
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
    ldDateTime_t *ld_date_time;

    if (dt == 0 || year == 0 || month == 0 || day == 0) {
        return -1;
    }

    ld_date_time = picoui_date_time_get_ld((struct picoui_date_time *)dt);
    if (ld_date_time == NULL) {
        return -1;
    }

    ((struct picoui_date_time *)dt)->year = ld_date_time->year;
    ((struct picoui_date_time *)dt)->month = ld_date_time->month;
    ((struct picoui_date_time *)dt)->day = ld_date_time->day;
    *year = ((struct picoui_date_time *)dt)->year;
    *month = ((struct picoui_date_time *)dt)->month;
    *day = ((struct picoui_date_time *)dt)->day;
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
    ldDateTime_t *ld_date_time;

    if (dt == 0 || hour == 0 || minute == 0 || second == 0) {
        return -1;
    }

    ld_date_time = picoui_date_time_get_ld((struct picoui_date_time *)dt);
    if (ld_date_time == NULL) {
        return -1;
    }

    ((struct picoui_date_time *)dt)->hour = ld_date_time->hour;
    ((struct picoui_date_time *)dt)->minute = ld_date_time->minute;
    ((struct picoui_date_time *)dt)->second = ld_date_time->second;
    *hour = ((struct picoui_date_time *)dt)->hour;
    *minute = ((struct picoui_date_time *)dt)->minute;
    *second = ((struct picoui_date_time *)dt)->second;
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
    ldDateTime_t *ld_date_time;

    if (dt == 0) {
        return -1;
    }

    ld_date_time = picoui_date_time_get_ld((struct picoui_date_time *)dt);
    if (ld_date_time == NULL) {
        return -1;
    }

    ((struct picoui_date_time *)dt)->transparent = ld_date_time->isTransparent ? 1 : 0;
    return ((struct picoui_date_time *)dt)->transparent;
}

/**
 * @brief Get use system time of date time widget
 *
 * @param[in] dt dt
 * @return -1 on failure
 */

int picoui_date_time_get_use_system_time(const struct picoui_date_time *dt)
{
    ldDateTime_t *ld_date_time;

    if (dt == 0) {
        return -1;
    }

    ld_date_time = picoui_date_time_get_ld((struct picoui_date_time *)dt);
    if (ld_date_time == NULL) {
        return -1;
    }

    ((struct picoui_date_time *)dt)->use_system_time = ld_date_time->isAutoSysTime ? 1 : 0;
    return ((struct picoui_date_time *)dt)->use_system_time;
}
