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

#ifndef TINYUI_DATE_TIME_H
#define TINYUI_DATE_TIME_H

#include "tinyui_config.h"
#if !TINYUI_ENABLE_DATE_TIME
#  ifndef TINYUI_INTERNAL_FEATURE_HEADER
#  error "TINYUI_ENABLE_DATE_TIME is disabled"
#  endif
#endif

#include "layout/layout.h"

#include "core/obj.h"
#include <stdint.h>

struct tinyui_font;

typedef enum tinyui_date_time_field {
    TINYUI_DATE_TIME_FIELD_ID = UINT32_C(1) << 0,
    TINYUI_DATE_TIME_FIELD_STYLE_CLASS = UINT32_C(1) << 1,
    TINYUI_DATE_TIME_FIELD_USER_DATA = UINT32_C(1) << 2,
    TINYUI_DATE_TIME_FIELD_FONT = UINT32_C(1) << 3,
    TINYUI_DATE_TIME_FIELD_FORMAT = UINT32_C(1) << 4,
    TINYUI_DATE_TIME_FIELD_TEXT_COLOR = UINT32_C(1) << 5,
    TINYUI_DATE_TIME_FIELD_BG_COLOR = UINT32_C(1) << 6,
    TINYUI_DATE_TIME_FIELD_ALIGN = UINT32_C(1) << 7,
    TINYUI_DATE_TIME_FIELD_TRANSPARENT = UINT32_C(1) << 8,
    TINYUI_DATE_TIME_FIELD_YEAR = UINT32_C(1) << 9,
    TINYUI_DATE_TIME_FIELD_MONTH = UINT32_C(1) << 10,
    TINYUI_DATE_TIME_FIELD_DAY = UINT32_C(1) << 11,
    TINYUI_DATE_TIME_FIELD_HOUR = UINT32_C(1) << 12,
    TINYUI_DATE_TIME_FIELD_MINUTE = UINT32_C(1) << 13,
    TINYUI_DATE_TIME_FIELD_SECOND = UINT32_C(1) << 14,
} tinyui_date_time_field_t;

typedef struct tinyui_date_time_props {
    uint32_t fields;
    uint16_t id;
    const char *style_class;
    void *user_data;
    const struct tinyui_font *font;
    const char *format;
    unsigned int text_color;
    unsigned int bg_color;
    enum tinyui_align align;
    int transparent;
    int year;
    int month;
    int day;
    int hour;
    int minute;
    int second;
} tinyui_date_time_props_t;

tinyui_obj_t *tinyui_date_time_create(tinyui_obj_t *parent);

tinyui_obj_t *tinyui_date_time_create_with_props(tinyui_obj_t *parent,
                                               const tinyui_date_time_props_t *props);

int tinyui_date_time_set_format(tinyui_obj_t *dt, const char *format);

int tinyui_date_time_set_date(tinyui_obj_t *dt, int year, int month, int day);

int tinyui_date_time_set_time(tinyui_obj_t *dt, int hour, int minute, int second);

int tinyui_date_time_set_font(tinyui_obj_t *dt, const struct tinyui_font *font);

int tinyui_date_time_set_text_color(tinyui_obj_t *dt, unsigned int rgb);

int tinyui_date_time_set_background_color(tinyui_obj_t *dt, unsigned int rgb);

int tinyui_date_time_set_bg_color(tinyui_obj_t *dt, unsigned int rgb);

int tinyui_date_time_set_align(tinyui_obj_t *dt, enum tinyui_align align);

int tinyui_date_time_set_transparent(tinyui_obj_t *dt, int transparent);

int tinyui_date_time_set_use_system_time(tinyui_obj_t *dt, int enabled);

const char *tinyui_date_time_get_format(const tinyui_obj_t *dt);

int tinyui_date_time_get_date(const tinyui_obj_t *dt, int *year, int *month, int *day);

int tinyui_date_time_get_time(const tinyui_obj_t *dt, int *hour, int *minute, int *second);

int tinyui_date_time_get_transparent(const tinyui_obj_t *dt);

int tinyui_date_time_get_use_system_time(const tinyui_obj_t *dt);

#endif /* TINYUI_DATE_TIME_H */
