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
#include "widgets/date_time.h"
#include "internal/widget_legacy.h"
#include "../core/runtime_bridge.h"
#include "../../../src/gui/ldDateTime.h"

#include <stdlib.h>
#include <string.h>


static struct tinyui_date_time *tinyui_date_time_as_date_time(tinyui_obj_t *obj)
{
    struct tinyui_widget *w = (struct tinyui_widget *)(void *)obj;
    if (w == 0 || !tinyui_runtime_internal_widget_is_kind(w, TINYUI_BACKEND_WIDGET_DATE_TIME)) {
        return 0;
    }
    return (struct tinyui_date_time *)w;
}

static const struct tinyui_date_time *tinyui_date_time_as_date_time_const(const tinyui_obj_t *obj)
{
    const struct tinyui_widget *w = (const struct tinyui_widget *)(const void *)obj;
    if (w == 0 || !tinyui_runtime_internal_widget_is_kind(w, TINYUI_BACKEND_WIDGET_DATE_TIME)) {
        return 0;
    }
    return (const struct tinyui_date_time *)w;
}

/* ---- test seam state ---- */


static arm_2d_font_t *tinyui_date_time_default_font(void)
{
    return tinyui_resolve_ld_font(0, 12);
}

static arm_2d_font_t *tinyui_date_time_resolve_font(const struct tinyui_font *font)
{
    return font == 0 ? tinyui_date_time_default_font() : tinyui_resolve_ld_font(font, 12);
}

static ldDateTime_t *tinyui_date_time_backend(struct tinyui_date_time *dt)
{
    if (dt == 0 || dt->widget.ld_widget == 0
        || dt->widget.kind != TINYUI_BACKEND_WIDGET_DATE_TIME) {
        return 0;
    }
    return (ldDateTime_t *)dt->widget.ld_widget;
}

static int tinyui_date_time_props_are_valid(const tinyui_date_time_props_t *props)
{
    return props != 0
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

static void *tinyui_runtime_internal_date_time_ld_init(void *ctx,
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
                           tinyui_date_time_default_font());
}

/**
 * @brief Create date time widget
 *
 * @param[in] parent Parent widget
 * @param[in] id Widget identifier string
 * @return Pointer to the object on success, NULL on failure
 */

tinyui_obj_t *tinyui_date_time_create(tinyui_obj_t *parent)
{
    struct tinyui_widget *parent_w = (struct tinyui_widget *)(void *)parent;
    const char *id = "date_time";
    if (parent_w == 0) { return 0; }

    struct tinyui_date_time *dt;
    ldDateTime_t *ld_date_time;

    if (parent_w == 0 || id == 0 || parent_w->ld_widget == 0) {
        return 0;
    }
    dt = (struct tinyui_date_time *)tinyui_runtime_internal_widget_create_leaf(parent_w,
                                                              TINYUI_BACKEND_WIDGET_DATE_TIME,
                                                              tinyui_runtime_internal_date_time_ld_init,
                                                              0,
                                                              sizeof(*dt));
    if (dt == 0) {
        return 0;
    }
    dt->id = id;
    if (tinyui_date_time_set_font(dt, NULL) != 0) {
        tinyui_runtime_internal_widget_destroy_common(&dt->widget);
        return 0;
    }
    ld_date_time = tinyui_date_time_backend(dt);
    if (ld_date_time == NULL) {
        tinyui_runtime_internal_widget_destroy_common(&dt->widget);
        return 0;
    }
    dt->format = (const char *)ld_date_time->formatStr;
    dt->year = ld_date_time->year;
    dt->month = ld_date_time->month;
    dt->day = ld_date_time->day;
    dt->hour = ld_date_time->hour;
    dt->minute = ld_date_time->minute;
    dt->second = ld_date_time->second;
    dt->transparent = ld_date_time->isTransparent ? 1 : 0;
    dt->use_system_time = ld_date_time->isAutoSysTime ? 1 : 0;
    return (tinyui_obj_t *)dt;
}

/**
 * @brief date time init
 *
 * @param[in] parent Parent widget
 * @param[in] id Widget identifier string
 * @return Pointer to the object
 */

struct tinyui_date_time *tinyui_runtime_internal_date_time_init(struct tinyui_widget *parent, const char *id)
{
    return tinyui_date_time_create(parent);
}

/**
 * @brief Create date time widget with properties
 *
 * @param[in] parent Parent widget
 * @param[in] props Properties structure
 * @return Pointer to the object on success, NULL on failure
 */

tinyui_obj_t *tinyui_date_time_create_with_props(tinyui_obj_t *parent,
                                             const tinyui_date_time_props_t *props)
{
    tinyui_obj_t *obj;
    struct tinyui_date_time *dt;

    if (props == 0) {
        return tinyui_date_time_create(parent);
    }

    obj = tinyui_date_time_create(parent);
    if (obj == 0) {
        return 0;
    }
    dt = (struct tinyui_date_time *)(void *)obj;

    if ((props->fields & TINYUI_DATE_TIME_FIELD_ID) != 0) {
        (void)props->id;
    }
    if ((props->fields & TINYUI_DATE_TIME_FIELD_USER_DATA) != 0) {
    if (tinyui_runtime_internal_widget_set_user_data(&dt->widget, props->user_data) != 0) {
        (void)tinyui_obj_delete((tinyui_obj_t *)dt);
        return 0;
    }
    }
    if ((props->fields & TINYUI_DATE_TIME_FIELD_STYLE_CLASS) != 0) {
    if (tinyui_runtime_internal_widget_set_style_class(&dt->widget, props->style_class) != 0) {
        (void)tinyui_obj_delete((tinyui_obj_t *)dt);
        return 0;
    }
    }
    if ((props->fields & TINYUI_DATE_TIME_FIELD_FONT) != 0) {
    if (tinyui_date_time_set_font((tinyui_obj_t *)dt, props->font) != 0) {
        (void)tinyui_obj_delete((tinyui_obj_t *)dt);
        return 0;
    }
    }
    if ((props->fields & TINYUI_DATE_TIME_FIELD_FORMAT) != 0) {
    if (tinyui_date_time_set_format((tinyui_obj_t *)dt, props->format) != 0) {
        (void)tinyui_obj_delete((tinyui_obj_t *)dt);
        return 0;
    }
    }
    if ((props->fields & TINYUI_DATE_TIME_FIELD_TEXT_COLOR) != 0) {
    if (tinyui_date_time_set_text_color((tinyui_obj_t *)dt, props->text_color) != 0) {
        (void)tinyui_obj_delete((tinyui_obj_t *)dt);
        return 0;
    }
    }
    if ((props->fields & TINYUI_DATE_TIME_FIELD_BG_COLOR) != 0) {
    if (tinyui_date_time_set_bg_color((tinyui_obj_t *)dt, props->bg_color) != 0) {
        (void)tinyui_obj_delete((tinyui_obj_t *)dt);
        return 0;
    }
    }
    if ((props->fields & TINYUI_DATE_TIME_FIELD_ALIGN) != 0) {
    if (tinyui_date_time_set_align((tinyui_obj_t *)dt, props->align) != 0) {
        (void)tinyui_obj_delete((tinyui_obj_t *)dt);
        return 0;
    }
    }
    if ((props->fields & TINYUI_DATE_TIME_FIELD_TRANSPARENT) != 0) {
    if (tinyui_date_time_set_transparent((tinyui_obj_t *)dt, props->transparent) != 0) {
        (void)tinyui_obj_delete((tinyui_obj_t *)dt);
        return 0;
    }
    }
    if ((props->fields & TINYUI_DATE_TIME_FIELD_YEAR) != 0 ||
        (props->fields & TINYUI_DATE_TIME_FIELD_MONTH) != 0 ||
        (props->fields & TINYUI_DATE_TIME_FIELD_DAY) != 0) {
        int y = 0;
        int m = 0;
        int d = 0;
        if (tinyui_date_time_get_date((const tinyui_obj_t *)dt, &y, &m, &d) != 0) {
            (void)tinyui_obj_delete((tinyui_obj_t *)dt);
            return 0;
        }
        if ((props->fields & TINYUI_DATE_TIME_FIELD_YEAR) != 0) {
            y = props->year;
        }
        if ((props->fields & TINYUI_DATE_TIME_FIELD_MONTH) != 0) {
            m = props->month;
        }
        if ((props->fields & TINYUI_DATE_TIME_FIELD_DAY) != 0) {
            d = props->day;
        }
            if (tinyui_date_time_set_date((tinyui_obj_t *)dt, y, m, d) != 0) {
                (void)tinyui_obj_delete((tinyui_obj_t *)dt);
                return 0;
            }
    }
    if ((props->fields & TINYUI_DATE_TIME_FIELD_HOUR) != 0 ||
        (props->fields & TINYUI_DATE_TIME_FIELD_MINUTE) != 0 ||
        (props->fields & TINYUI_DATE_TIME_FIELD_SECOND) != 0) {
        int hh = 0;
        int mm = 0;
        int ss = 0;
        if (tinyui_date_time_get_time((const tinyui_obj_t *)dt, &hh, &mm, &ss) != 0) {
            (void)tinyui_obj_delete((tinyui_obj_t *)dt);
            return 0;
        }
        if ((props->fields & TINYUI_DATE_TIME_FIELD_HOUR) != 0) {
            hh = props->hour;
        }
        if ((props->fields & TINYUI_DATE_TIME_FIELD_MINUTE) != 0) {
            mm = props->minute;
        }
        if ((props->fields & TINYUI_DATE_TIME_FIELD_SECOND) != 0) {
            ss = props->second;
        }
            if (tinyui_date_time_set_time((tinyui_obj_t *)dt, hh, mm, ss) != 0) {
                (void)tinyui_obj_delete((tinyui_obj_t *)dt);
                return 0;
            }
    }

    return obj;
}



/**
 * @brief Set format of date time widget
 *
 * @param[in] dt dt
 * @param[in] format Format string
 * @return 0 on success, -1 on failure
 */

int tinyui_date_time_set_format(tinyui_obj_t *dt_obj, const char *format)
{
    struct tinyui_date_time *dt = tinyui_date_time_as_date_time(dt_obj);
    if (dt == 0) { return -1; }

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

int tinyui_date_time_set_date(tinyui_obj_t *dt_obj, int year, int month, int day)
{
    struct tinyui_date_time *dt = tinyui_date_time_as_date_time(dt_obj);
    if (dt == 0) { return -1; }

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

int tinyui_date_time_set_time(tinyui_obj_t *dt_obj, int hour, int minute, int second)
{
    struct tinyui_date_time *dt = tinyui_date_time_as_date_time(dt_obj);
    if (dt == 0) { return -1; }

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

int tinyui_date_time_set_font(tinyui_obj_t *dt_obj, const struct tinyui_font *font)
{
    struct tinyui_date_time *dt = tinyui_date_time_as_date_time(dt_obj);
    if (dt == 0) { return -1; }

    ldDateTime_t *ld_date_time;
    arm_2d_font_t *resolved_font;

    if (dt == 0) {
        return -1;
    }

    ld_date_time = tinyui_date_time_backend(dt);
    if (ld_date_time == NULL) {
        return -1;
    }

    resolved_font = tinyui_date_time_resolve_font(font);
    if (resolved_font == NULL) {
        return -1;
    }

    ld_date_time->ptFont = resolved_font;
    dt->widget.font = font;
    return 0;
}

/**
 * @brief Set text color of date time widget
 *
 * @param[in] dt dt
 * @param[in] rgb RGB color value (0xRRGGBB)
 * @return 0 on success, -1 on failure
 */

int tinyui_date_time_set_text_color(tinyui_obj_t *dt_obj, unsigned int rgb)
{
    struct tinyui_date_time *dt = tinyui_date_time_as_date_time(dt_obj);
    if (dt == 0) { return -1; }

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

int tinyui_date_time_set_bg_color(tinyui_obj_t *dt_obj, unsigned int rgb)
{
    struct tinyui_date_time *dt = tinyui_date_time_as_date_time(dt_obj);
    if (dt == 0) { return -1; }

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

int tinyui_date_time_set_background_color(tinyui_obj_t *dt_obj, unsigned int rgb)
{
    struct tinyui_date_time *dt = tinyui_date_time_as_date_time(dt_obj);
    if (dt == 0) { return -1; }

    return tinyui_date_time_set_bg_color((tinyui_obj_t *)dt, rgb);
}

/**
 * @brief Set align of date time widget
 *
 * @param[in] dt dt
 * @param[in] align align
 * @return 0 on success, -1 on failure
 */

int tinyui_date_time_set_align(tinyui_obj_t *dt_obj, enum tinyui_align align)
{
    struct tinyui_date_time *dt = tinyui_date_time_as_date_time(dt_obj);
    if (dt == 0) { return -1; }

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

int tinyui_date_time_set_transparent(tinyui_obj_t *dt_obj, int transparent)
{
    struct tinyui_date_time *dt = tinyui_date_time_as_date_time(dt_obj);
    if (dt == 0) { return -1; }

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

int tinyui_date_time_set_use_system_time(tinyui_obj_t *dt_obj, int enabled)
{
    struct tinyui_date_time *dt = tinyui_date_time_as_date_time(dt_obj);
    if (dt == 0) { return -1; }

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

const char * tinyui_date_time_get_format(const tinyui_obj_t *dt_obj)
{
    const struct tinyui_date_time *dt = tinyui_date_time_as_date_time_const(dt_obj);
    if (dt == 0) { return 0; }

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

int tinyui_date_time_get_date(const tinyui_obj_t *dt_obj, int *year, int *month, int *day)
{
    const struct tinyui_date_time *dt = tinyui_date_time_as_date_time_const(dt_obj);
    if (dt == 0) { return -1; }

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

int tinyui_date_time_get_time(const tinyui_obj_t *dt_obj, int *hour, int *minute, int *second)
{
    const struct tinyui_date_time *dt = tinyui_date_time_as_date_time_const(dt_obj);
    if (dt == 0) { return -1; }

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

int tinyui_date_time_get_transparent(const tinyui_obj_t *dt_obj)
{
    const struct tinyui_date_time *dt = tinyui_date_time_as_date_time_const(dt_obj);
    if (dt == 0) { return -1; }

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

int tinyui_date_time_get_use_system_time(const tinyui_obj_t *dt_obj)
{
    const struct tinyui_date_time *dt = tinyui_date_time_as_date_time_const(dt_obj);
    if (dt == 0) { return -1; }

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
