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

#ifndef PICOUI_DATE_TIME_H
#define PICOUI_DATE_TIME_H

#include "picoui/widget.h"

struct picoui_date_time;

struct picoui_date_time_props {
    const char *id;
    const char *style_class;
    void *user_data;
    const char *format;
    unsigned int text_color;
    unsigned int bg_color;
    enum picoui_align align;
    int transparent;
    int year;
    int month;
    int day;
    int hour;
    int minute;
    int second;
};

/**
 * @brief Create date time widget
 *
 * @param[in] parent Parent widget
 * @param[in] id Widget identifier string
 * @return Pointer to the object on success, NULL on failure
 */

struct picoui_date_time *picoui_date_time_create(struct picoui_widget *parent, const char *id);

/**
 * @brief Create date time widget with properties
 *
 * @param[in] parent Parent widget
 * @param[in] props Properties structure
 * @return Pointer to the object on success, NULL on failure
 */

struct picoui_date_time *picoui_date_time_create_with_props(
    struct picoui_widget *parent,
    const struct picoui_date_time_props *props);

/**
 * @brief date time init
 *
 * @param[in] parent Parent widget
 * @param[in] id Widget identifier string
 * @return Pointer to the object on success, NULL on failure
 */

struct picoui_date_time *picoui_date_time_init(struct picoui_widget *parent, const char *id);

/**
 * @brief Set format of date time widget
 *
 * @param[in] dt dt
 * @param[in] format Format string
 * @return 0 on success, -1 on failure
 */

int picoui_date_time_set_format(struct picoui_date_time *dt, const char *format);

/**
 * @brief Set date of date time widget
 *
 * @param[in] dt dt
 * @param[in] year year
 * @param[in] month month
 * @param[in] day day
 * @return 0 on success, -1 on failure
 */

int picoui_date_time_set_date(struct picoui_date_time *dt, int year, int month, int day);

/**
 * @brief Set time of date time widget
 *
 * @param[in] dt dt
 * @param[in] hour hour
 * @param[in] minute minute
 * @param[in] second second
 * @return 0 on success, -1 on failure
 */

int picoui_date_time_set_time(struct picoui_date_time *dt, int hour, int minute, int second);

/**
 * @brief Set text color of date time widget
 *
 * @param[in] dt dt
 * @param[in] rgb RGB color value (0xRRGGBB)
 * @return 0 on success, -1 on failure
 */

int picoui_date_time_set_text_color(struct picoui_date_time *dt, unsigned int rgb);

/**
 * @brief Set background color of date time widget
 *
 * @param[in] dt dt
 * @param[in] rgb RGB color value (0xRRGGBB)
 * @return 0 on success, -1 on failure
 */

int picoui_date_time_set_background_color(struct picoui_date_time *dt, unsigned int rgb);

/**
 * @brief Set bg color of date time widget
 *
 * @param[in] dt dt
 * @param[in] rgb RGB color value (0xRRGGBB)
 * @return 0 on success, -1 on failure
 */

int picoui_date_time_set_bg_color(struct picoui_date_time *dt, unsigned int rgb);

/**
 * @brief Set align of date time widget
 *
 * @param[in] dt dt
 * @param[in] align align
 * @return 0 on success, -1 on failure
 */

int picoui_date_time_set_align(struct picoui_date_time *dt, enum picoui_align align);

/**
 * @brief Set transparent of date time widget
 *
 * @param[in] dt dt
 * @param[in] transparent transparent
 * @return 0 on success, -1 on failure
 */

int picoui_date_time_set_transparent(struct picoui_date_time *dt, int transparent);

/**
 * @brief Set use system time of date time widget
 *
 * @param[in] dt dt
 * @param[in] enabled Enable state
 * @return 0 on success, -1 on failure
 */

int picoui_date_time_set_use_system_time(struct picoui_date_time *dt, int enabled);

/**
 * @brief Get format of date time widget
 *
 * @param[in] dt dt
 */

const char *picoui_date_time_get_format(const struct picoui_date_time *dt);

/**
 * @brief Get date of date time widget
 *
 * @param[in] dt dt
 * @param[in] year year
 * @param[in] month month
 * @param[in] day day
 * @return The property value, negative on error
 */

int picoui_date_time_get_date(const struct picoui_date_time *dt, int *year, int *month, int *day);

/**
 * @brief Get time of date time widget
 *
 * @param[in] dt dt
 * @param[in] hour hour
 * @param[in] minute minute
 * @param[in] second second
 * @return The property value, negative on error
 */

int picoui_date_time_get_time(const struct picoui_date_time *dt, int *hour, int *minute, int *second);

/**
 * @brief Get transparent of date time widget
 *
 * @param[in] dt dt
 * @return The property value, negative on error
 */

int picoui_date_time_get_transparent(const struct picoui_date_time *dt);

/**
 * @brief Get use system time of date time widget
 *
 * @param[in] dt dt
 * @return The property value, negative on error
 */

int picoui_date_time_get_use_system_time(const struct picoui_date_time *dt);

#endif
