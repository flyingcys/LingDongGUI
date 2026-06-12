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

#ifndef PICOUI_CALENDAR_H
#define PICOUI_CALENDAR_H

#include "../widget.h"

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

/**
 * @brief Create calendar widget
 *
 * @param[in] parent Parent widget
 * @param[in] id Widget identifier string
 * @return Pointer to the object on success, NULL on failure
 */

struct picoui_calendar *picoui_calendar_create(struct picoui_window *parent, const char *id);

/**
 * @brief Create calendar widget with properties
 *
 * @param[in] parent Parent widget
 * @param[in] props Properties structure
 * @return Pointer to the object on success, NULL on failure
 */

struct picoui_calendar *picoui_calendar_create_with_props(struct picoui_window *parent,
                                                          const struct picoui_calendar_props *props);

/**
 * @brief calendar init
 *
 * @param[in] parent Parent widget
 * @param[in] id Widget identifier string
 * @return Pointer to the object on success, NULL on failure
 */

struct picoui_calendar *picoui_calendar_init(struct picoui_window *parent, const char *id);

/**
 * @brief Set date of calendar widget
 *
 * @param[in] calendar Calendar widget instance
 * @param[in] year year
 * @param[in] month month
 * @param[in] day day
 * @return 0 on success, -1 on failure
 */

int picoui_calendar_set_date(struct picoui_calendar *calendar, int year, int month, int day);

/**
 * @brief Set use system date of calendar widget
 *
 * @param[in] calendar Calendar widget instance
 * @param[in] enabled Enable state
 * @return 0 on success, -1 on failure
 */

int picoui_calendar_set_use_system_date(struct picoui_calendar *calendar, int enabled);

/**
 * @brief Set day names of calendar widget
 *
 * @param[in] calendar Calendar widget instance
 * @param[in] day_names[7 day names[7
 * @return 0 on success, -1 on failure
 */

int picoui_calendar_set_day_names(struct picoui_calendar *calendar, const char *const day_names[7]);

/**
 * @brief Get date of calendar widget
 *
 * @param[in] calendar Calendar widget instance
 * @param[in] year year
 * @param[in] month month
 * @param[in] day day
 * @return The property value, negative on error
 */

int picoui_calendar_get_date(const struct picoui_calendar *calendar,
                             int *year,
                             int *month,
                             int *day);

/**
 * @brief Get use system date of calendar widget
 *
 * @param[in] calendar Calendar widget instance
 * @return The property value, negative on error
 */

int picoui_calendar_get_use_system_date(const struct picoui_calendar *calendar);

/**
 * @brief Set header visible of calendar widget
 *
 * @param[in] calendar Calendar widget instance
 * @param[in] visible Visibility state
 * @return 0 on success, -1 on failure
 */

int picoui_calendar_set_header_visible(struct picoui_calendar *calendar, int visible);

/**
 * @brief Get header visible of calendar widget
 *
 * @param[in] calendar Calendar widget instance
 * @return The property value, negative on error
 */

int picoui_calendar_get_header_visible(const struct picoui_calendar *calendar);

/**
 * @brief Set header format of calendar widget
 *
 * @param[in] calendar Calendar widget instance
 * @param[in] format Format string
 * @return 0 on success, -1 on failure
 */

int picoui_calendar_set_header_format(struct picoui_calendar *calendar, const char *format);

/**
 * @brief Set bg color of calendar widget
 *
 * @param[in] calendar Calendar widget instance
 * @param[in] rgb RGB color value (0xRRGGBB)
 * @return 0 on success, -1 on failure
 */

int picoui_calendar_set_bg_color(struct picoui_calendar *calendar, unsigned int rgb);

/**
 * @brief Set item color of calendar widget
 *
 * @param[in] calendar Calendar widget instance
 * @param[in] rgb RGB color value (0xRRGGBB)
 * @return 0 on success, -1 on failure
 */

int picoui_calendar_set_item_color(struct picoui_calendar *calendar, unsigned int rgb);

/**
 * @brief Set text color of calendar widget
 *
 * @param[in] calendar Calendar widget instance
 * @param[in] rgb RGB color value (0xRRGGBB)
 * @return 0 on success, -1 on failure
 */

int picoui_calendar_set_text_color(struct picoui_calendar *calendar, unsigned int rgb);

/**
 * @brief Get header format of calendar widget
 *
 * @param[in] calendar Calendar widget instance
 */

const char *picoui_calendar_get_header_format(const struct picoui_calendar *calendar);

/**
 * @brief Get grid value of calendar widget
 *
 * @param[in] calendar Calendar widget instance
 * @param[in] week week
 * @param[in] weekday weekday
 * @return The property value, negative on error
 */

int picoui_calendar_get_grid_value(const struct picoui_calendar *calendar, int week, int weekday);

/**
 * @brief calendar is current month cell
 *
 * @param[in] calendar Calendar widget instance
 * @param[in] week week
 * @param[in] weekday weekday
 * @return 0 on success, -1 on failure
 */

int picoui_calendar_is_current_month_cell(const struct picoui_calendar *calendar, int week, int weekday);

#endif
