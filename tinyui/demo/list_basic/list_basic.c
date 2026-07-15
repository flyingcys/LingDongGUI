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

#include "list_basic/list_basic.h"
#include "tinyui.h"

tinyui_result_t tinyui_demo_list_basic_build(tinyui_obj_t *screen)
{
    tinyui_obj_t *title;
    tinyui_obj_t *list;
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
    result = tinyui_flex_set_gap(screen, 8, 8);
    if (result != TINYUI_OK) {
        return result;
    }

    title = tinyui_label_create(screen);
    list = tinyui_list_create(screen);
    if (title == NULL || list == NULL) {
        return TINYUI_ERROR_NO_MEMORY;
    }

    if (tinyui_label_set_text(title, "List") != 0
        || tinyui_list_add_item(list, "item_wifi", "Wi-Fi") != 0
        || tinyui_list_add_item(list, "item_bluetooth", "Bluetooth") != 0
        || tinyui_list_add_item(list, "item_display", "Display") != 0
        || tinyui_list_set_selected_index(list, 0) != 0) {
        return TINYUI_ERROR_BACKEND;
    }

    return TINYUI_OK;
}
