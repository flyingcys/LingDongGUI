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

#include "combo_box_basic/combo_box_basic.h"
#include "tinyui.h"

#include <string.h>

tinyui_result_t tinyui_demo_combo_box_basic_build(tinyui_obj_t *screen)
{
    tinyui_obj_t *title;
    tinyui_obj_t *combo_box;
    tinyui_combo_box_props_t combo_props;
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

    memset(&combo_props, 0, sizeof(combo_props));
    combo_props.fields = TINYUI_COMBO_BOX_FIELD_WIDTH | TINYUI_COMBO_BOX_FIELD_HEIGHT;
    combo_props.width = 220;
    combo_props.height = 32;

    title = tinyui_label_create(screen);
    combo_box = tinyui_combo_box_create_with_props(screen, &combo_props);
    if (title == NULL || combo_box == NULL) {
        return TINYUI_ERROR_NO_MEMORY;
    }

    if (tinyui_label_set_text(title, "Combo Box") != 0
        || tinyui_combo_box_add_item(combo_box, "wifi", "Wi-Fi") != 0
        || tinyui_combo_box_add_item(combo_box, "bluetooth", "Bluetooth") != 0
        || tinyui_combo_box_add_item(combo_box, "display", "Display") != 0
        || tinyui_combo_box_set_selected_index(combo_box, 1) != 0) {
        return TINYUI_ERROR_BACKEND;
    }

    return TINYUI_OK;
}
