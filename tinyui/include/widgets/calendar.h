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

#ifndef TINYUI_CALENDAR_H
#define TINYUI_CALENDAR_H

#include "core/obj.h"
#include <stdint.h>

typedef enum tinyui_calendar_field {
    TINYUI_CALENDAR_FIELD_ID = UINT32_C(1) << 0,
    TINYUI_CALENDAR_FIELD_YEAR = UINT32_C(1) << 1,
    TINYUI_CALENDAR_FIELD_MONTH = UINT32_C(1) << 2,
    TINYUI_CALENDAR_FIELD_DAY = UINT32_C(1) << 3,
    TINYUI_CALENDAR_FIELD_WIDTH = UINT32_C(1) << 4,
    TINYUI_CALENDAR_FIELD_HEIGHT = UINT32_C(1) << 5,
    TINYUI_CALENDAR_FIELD_SHOW_HEADER = UINT32_C(1) << 6,
    TINYUI_CALENDAR_FIELD_HEADER_FORMAT = UINT32_C(1) << 7,
    TINYUI_CALENDAR_FIELD_STYLE_CLASS = UINT32_C(1) << 8,
    TINYUI_CALENDAR_FIELD_USER_DATA = UINT32_C(1) << 9,
} tinyui_calendar_field_t;

typedef struct tinyui_calendar_props {
    uint32_t fields;
    uint16_t id;
    int year;
    int month;
    int day;
    int width;
    int height;
    int show_header;
    const char *header_format;
    const char *style_class;
    void *user_data;
} tinyui_calendar_props_t;

tinyui_obj_t *tinyui_calendar_create(tinyui_obj_t *parent);

tinyui_obj_t *tinyui_calendar_create_with_props(tinyui_obj_t *parent,
                                               const tinyui_calendar_props_t *props);

int tinyui_calendar_set_date(tinyui_obj_t *calendar, int year, int month, int day);

int tinyui_calendar_set_use_system_date(tinyui_obj_t *calendar, int enabled);

int tinyui_calendar_set_auto_sys_date(tinyui_obj_t *calendar, int enabled);

int tinyui_calendar_set_day_names(tinyui_obj_t *calendar, const char *const day_names[7]);

int tinyui_calendar_get_date(const tinyui_obj_t *calendar, int *year, int *month, int *day);

int tinyui_calendar_get_use_system_date(const tinyui_obj_t *calendar);

int tinyui_calendar_set_header_visible(tinyui_obj_t *calendar, int visible);

int tinyui_calendar_get_header_visible(const tinyui_obj_t *calendar);

int tinyui_calendar_set_header_format(tinyui_obj_t *calendar, const char *format);

int tinyui_calendar_set_bg_color(tinyui_obj_t *calendar, unsigned int rgb);

int tinyui_calendar_set_item_color(tinyui_obj_t *calendar, unsigned int rgb);

int tinyui_calendar_set_text_color(tinyui_obj_t *calendar, unsigned int rgb);

const char *tinyui_calendar_get_header_format(const tinyui_obj_t *calendar);

int tinyui_calendar_get_grid_value(const tinyui_obj_t *calendar, int week, int weekday);

int tinyui_calendar_is_current_month_cell(const tinyui_obj_t *calendar, int week, int weekday);

#endif /* TINYUI_CALENDAR_H */
