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
#include "date_time.h"
#include "widget.h"
#include "../core/runtime_bridge.h"
#include "../../../src/gui/ldDateTime.h"

#include <stdlib.h>

extern const arm_2d_a1_font_t ARM_2D_FONT_6x8;

/* ---- test seam state ---- */
static ld_scene_t *s_date_time_depose_scene = NULL;

static void tinyui_date_time_ld_depose_cb(void *ld_widget)
{
    if (s_date_time_depose_scene != NULL) {
        ldDateTime_depose(s_date_time_depose_scene, (ldDateTime_t *)ld_widget);
        s_date_time_depose_scene = NULL;
    }
}

static ldDateTime_t *tinyui_date_time_backend(struct tinyui_date_time *dt)
{
    if (dt == 0 || dt->widget.ld_widget == 0
        || dt->widget.kind != TINYUI_BACKEND_WIDGET_DATE_TIME) {
        return 0;
    }
    return (ldDateTime_t *)dt->widget.ld_widget;
}

static int tinyui_date_time_props_are_valid(const struct tinyui_date_time_props *props)
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

static void *tinyui_date_time_ld_init(void *ctx,
                                      struct ld_scene_t *scene,
                                      uint16_t name_id,
                                      uint16_t parent_name_id)
{
    (void)ctx;
    return ldDateTime_init(scene,
                           NULL,
                           name_id,
                           parent_name_id,
                           0,
                           0,
                           240,
                           32,
                           (arm_2d_font_t *)&ARM_2D_FONT_6x8);
}

/**
 * @brief Create date time widget
 *
 * @param[in] parent Parent widget
 * @param[in] id Widget identifier string
 * @return Pointer to the object on success, NULL on failure
 */

struct tinyui_date_time *tinyui_date_time_create(struct tinyui_widget *parent, const char *id)
{
    struct tinyui_date_time *dt;

    if (parent == 0 || id == 0 || parent->ld_widget == 0) {
        return 0;
    }
    dt = (struct tinyui_date_time *)tinyui_widget_create_leaf(parent,
                                                              TINYUI_BACKEND_WIDGET_DATE_TIME,
                                                              tinyui_date_time_ld_init,
                                                              0,
                                                              sizeof(*dt));
    if (dt == 0) {
        return 0;
    }
    dt->id = id;
    if (tinyui_date_time_set_format(dt, "yyyy-mm-dd hh:nn:ss") != 0
        || tinyui_date_time_set_text_color(dt, 0x000000U) != 0
        || tinyui_date_time_set_bg_color(dt, 0xFFFFFFU) != 0
        || tinyui_date_time_set_align(dt, TINYUI_ALIGN_CENTER) != 0
        || tinyui_date_time_set_transparent(dt, 0) != 0
        || tinyui_date_time_set_date(dt, 2026, 1, 1) != 0
        || tinyui_date_time_set_time(dt, 12, 0, 0) != 0) {
        s_date_time_depose_scene = dt->widget.owner != 0 ? dt->widget.owner->ld_scene : 0;
        tinyui_widget_destroy_common(&dt->widget, tinyui_date_time_ld_depose_cb);
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

struct tinyui_date_time *tinyui_date_time_init(struct tinyui_widget *parent, const char *id)
{
    return tinyui_date_time_create(parent, id);
}

/**
 * @brief Create date time widget with properties
 *
 * @param[in] parent Parent widget
 * @param[in] props Properties structure
 * @return Pointer to the object on success, NULL on failure
 */

struct tinyui_date_time *tinyui_date_time_create_with_props(
    struct tinyui_widget *parent,
    const struct tinyui_date_time_props *props)
{
    struct tinyui_date_time *dt;

    if (!tinyui_date_time_props_are_valid(props)) {
        return 0;
    }

    dt = tinyui_date_time_create(parent, props->id);
    if (dt == 0) {
        return 0;
    }

    if ((props->style_class != 0
            && tinyui_widget_set_style_class(&dt->widget, props->style_class) != 0)
        || tinyui_widget_set_user_data(&dt->widget, props->user_data) != 0
        || tinyui_date_time_set_format(dt, props->format) != 0
        || tinyui_date_time_set_text_color(dt, props->text_color) != 0
        || tinyui_date_time_set_bg_color(dt, props->bg_color) != 0
        || tinyui_date_time_set_align(dt, props->align) != 0
        || tinyui_date_time_set_transparent(dt, props->transparent) != 0
        || tinyui_date_time_set_date(dt, props->year, props->month, props->day) != 0
        || tinyui_date_time_set_time(dt, props->hour, props->minute, props->second) != 0) {
        s_date_time_depose_scene = dt->widget.owner != 0
                                       ? dt->widget.owner->ld_scene
                                       : 0;
        tinyui_widget_destroy_common(&dt->widget, tinyui_date_time_ld_depose_cb);
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

int tinyui_date_time_set_format(struct tinyui_date_time *dt, const char *format)
{
    ldDateTime_t *ld_date_time;

    if (dt == 0 || format == 0) {
        return -1;
    }

    ld_date_time = tinyui_date_time_backend(dt);
    if (ld_date_time == NULL) {
        return -1;
    }

    ldDateTimeSetFormat(ld_date_time, (const uint8_t *)format);
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

int tinyui_date_time_set_date(struct tinyui_date_time *dt, int year, int month, int day)
{
    ldDateTime_t *ld_date_time;

    if (dt == 0 || month < 1 || month > 12 || day < 1 || day > 31) {
        return -1;
    }

    ld_date_time = tinyui_date_time_backend(dt);
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

int tinyui_date_time_set_time(struct tinyui_date_time *dt, int hour, int minute, int second)
{
    ldDateTime_t *ld_date_time;

    if (dt == 0
        || hour < 0 || hour > 23
        || minute < 0 || minute > 59
        || second < 0 || second > 59) {
        return -1;
    }

    ld_date_time = tinyui_date_time_backend(dt);
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

int tinyui_date_time_set_text_color(struct tinyui_date_time *dt, unsigned int rgb)
{
    ldDateTime_t *ld_date_time;

    if (dt == 0 || rgb > 0xFFFFFFU) {
        return -1;
    }

    ld_date_time = tinyui_date_time_backend(dt);
    if (ld_date_time == NULL) {
        return -1;
    }

    ldDateTimeSetTextColor(ld_date_time, (ldColor)tinyui_rgb_to_ld_color(rgb));
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

int tinyui_date_time_set_bg_color(struct tinyui_date_time *dt, unsigned int rgb)
{
    ldDateTime_t *ld_date_time;

    if (dt == 0 || rgb > 0xFFFFFFU) {
        return -1;
    }

    ld_date_time = tinyui_date_time_backend(dt);
    if (ld_date_time == NULL) {
        return -1;
    }

    ldDateTimeSetBackgroundColor(ld_date_time, (ldColor)tinyui_rgb_to_ld_color(rgb));
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

int tinyui_date_time_set_background_color(struct tinyui_date_time *dt, unsigned int rgb)
{
    return tinyui_date_time_set_bg_color(dt, rgb);
}

/**
 * @brief Set align of date time widget
 *
 * @param[in] dt dt
 * @param[in] align align
 * @return 0 on success, -1 on failure
 */

int tinyui_date_time_set_align(struct tinyui_date_time *dt, enum tinyui_align align)
{
    ldDateTime_t *ld_date_time;

    if (dt == 0
        || (align != TINYUI_ALIGN_START
            && align != TINYUI_ALIGN_CENTER
            && align != TINYUI_ALIGN_END)) {
        return -1;
    }

    ld_date_time = tinyui_date_time_backend(dt);
    if (ld_date_time == NULL) {
        return -1;
    }

    ldDateTimeSetAlign(ld_date_time, (arm_2d_align_t)tinyui_align_to_arm2d(align));
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

int tinyui_date_time_set_transparent(struct tinyui_date_time *dt, int transparent)
{
    ldDateTime_t *ld_date_time;

    if (dt == 0) {
        return -1;
    }

    ld_date_time = tinyui_date_time_backend(dt);
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

int tinyui_date_time_set_use_system_time(struct tinyui_date_time *dt, int enabled)
{
    ldDateTime_t *ld_date_time;

    if (dt == 0) {
        return -1;
    }

    ld_date_time = tinyui_date_time_backend(dt);
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

const char *tinyui_date_time_get_format(const struct tinyui_date_time *dt)
{
    ldDateTime_t *ld_date_time;

    if (dt == 0) {
        return 0;
    }

    ld_date_time = tinyui_date_time_backend((struct tinyui_date_time *)dt);
    if (ld_date_time == NULL) {
        return 0;
    }

    ((struct tinyui_date_time *)dt)->format = (const char *)ld_date_time->formatStr;
    return ((struct tinyui_date_time *)dt)->format;
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

int tinyui_date_time_get_date(const struct tinyui_date_time *dt, int *year, int *month, int *day)
{
    ldDateTime_t *ld_date_time;

    if (dt == 0 || year == 0 || month == 0 || day == 0) {
        return -1;
    }

    ld_date_time = tinyui_date_time_backend((struct tinyui_date_time *)dt);
    if (ld_date_time == NULL) {
        return -1;
    }

    ((struct tinyui_date_time *)dt)->year = ld_date_time->year;
    ((struct tinyui_date_time *)dt)->month = ld_date_time->month;
    ((struct tinyui_date_time *)dt)->day = ld_date_time->day;
    *year = ((struct tinyui_date_time *)dt)->year;
    *month = ((struct tinyui_date_time *)dt)->month;
    *day = ((struct tinyui_date_time *)dt)->day;
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

int tinyui_date_time_get_time(const struct tinyui_date_time *dt, int *hour, int *minute, int *second)
{
    ldDateTime_t *ld_date_time;

    if (dt == 0 || hour == 0 || minute == 0 || second == 0) {
        return -1;
    }

    ld_date_time = tinyui_date_time_backend((struct tinyui_date_time *)dt);
    if (ld_date_time == NULL) {
        return -1;
    }

    ((struct tinyui_date_time *)dt)->hour = ld_date_time->hour;
    ((struct tinyui_date_time *)dt)->minute = ld_date_time->minute;
    ((struct tinyui_date_time *)dt)->second = ld_date_time->second;
    *hour = ((struct tinyui_date_time *)dt)->hour;
    *minute = ((struct tinyui_date_time *)dt)->minute;
    *second = ((struct tinyui_date_time *)dt)->second;
    return 0;
}

/**
 * @brief Get transparent of date time widget
 *
 * @param[in] dt dt
 * @return -1 on failure
 */

int tinyui_date_time_get_transparent(const struct tinyui_date_time *dt)
{
    ldDateTime_t *ld_date_time;

    if (dt == 0) {
        return -1;
    }

    ld_date_time = tinyui_date_time_backend((struct tinyui_date_time *)dt);
    if (ld_date_time == NULL) {
        return -1;
    }

    ((struct tinyui_date_time *)dt)->transparent = ld_date_time->isTransparent ? 1 : 0;
    return ((struct tinyui_date_time *)dt)->transparent;
}

/**
 * @brief Get use system time of date time widget
 *
 * @param[in] dt dt
 * @return -1 on failure
 */

int tinyui_date_time_get_use_system_time(const struct tinyui_date_time *dt)
{
    ldDateTime_t *ld_date_time;

    if (dt == 0) {
        return -1;
    }

    ld_date_time = tinyui_date_time_backend((struct tinyui_date_time *)dt);
    if (ld_date_time == NULL) {
        return -1;
    }

    ((struct tinyui_date_time *)dt)->use_system_time = ld_date_time->isAutoSysTime ? 1 : 0;
    return ((struct tinyui_date_time *)dt)->use_system_time;
}
