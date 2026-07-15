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

#include "calendar_basic/calendar_basic.h"
#include "tinyui.h"

#include <string.h>

tinyui_result_t tinyui_demo_calendar_basic_build(tinyui_obj_t *screen)
{
    tinyui_obj_t *title;
    tinyui_obj_t *calendar;
    tinyui_calendar_props_t calendar_props;
    tinyui_result_t result;

    if (screen == NULL) {
        return TINYUI_ERROR_INVALID_ARG;
    }

    result = tinyui_flex_set_flow(screen, TINYUI_FLEX_FLOW_COLUMN);
    if (result != TINYUI_OK) {
        return result;
    }
    result = tinyui_flex_set_align(screen,
                                   TINYUI_ALIGN_START,
                                   TINYUI_ALIGN_START,
                                   TINYUI_ALIGN_START);
    if (result != TINYUI_OK) {
        return result;
    }
    result = tinyui_flex_set_gap(screen, 12, 12);
    if (result != TINYUI_OK) {
        return result;
    }
    if (tinyui_window_set_padding(screen, 24, 24, 24, 24) != 0) {
        return TINYUI_ERROR_BACKEND;
    }

    memset(&calendar_props, 0, sizeof(calendar_props));
    calendar_props.fields = TINYUI_CALENDAR_FIELD_YEAR | TINYUI_CALENDAR_FIELD_MONTH
        | TINYUI_CALENDAR_FIELD_DAY | TINYUI_CALENDAR_FIELD_WIDTH
        | TINYUI_CALENDAR_FIELD_HEIGHT | TINYUI_CALENDAR_FIELD_SHOW_HEADER
        | TINYUI_CALENDAR_FIELD_HEADER_FORMAT;
    calendar_props.year = 2026;
    calendar_props.month = 6;
    calendar_props.day = 15;
    calendar_props.width = 280;
    calendar_props.height = 180;
    calendar_props.show_header = 1;
    calendar_props.header_format = "yyyy/mm/dd";

    title = tinyui_label_create(screen);
    calendar = tinyui_calendar_create_with_props(screen, &calendar_props);
    if (title == NULL || calendar == NULL) {
        return TINYUI_ERROR_NO_MEMORY;
    }

    if (tinyui_label_set_text(title, "Calendar") != 0) {
        return TINYUI_ERROR_BACKEND;
    }

    return TINYUI_OK;
}
