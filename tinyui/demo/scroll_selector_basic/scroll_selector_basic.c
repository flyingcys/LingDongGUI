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

#include "scroll_selector_basic/scroll_selector_basic.h"
#include "tinyui.h"

#include <string.h>

tinyui_result_t tinyui_demo_scroll_selector_basic_build(tinyui_obj_t *screen)
{
    tinyui_obj_t *title;
    tinyui_obj_t *hint;
    tinyui_obj_t *scroll_selector;
    tinyui_scroll_selector_props_t selector_props;
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

    memset(&selector_props, 0, sizeof(selector_props));
    selector_props.fields = TINYUI_SCROLL_SELECTOR_FIELD_WIDTH
        | TINYUI_SCROLL_SELECTOR_FIELD_HEIGHT;
    selector_props.width = 220;
    selector_props.height = 120;

    title = tinyui_label_create(screen);
    hint = tinyui_label_create(screen);
    scroll_selector = tinyui_scroll_selector_create_with_props(screen, &selector_props);
    if (title == NULL || hint == NULL || scroll_selector == NULL) {
        return TINYUI_ERROR_NO_MEMORY;
    }

    if (tinyui_label_set_text(title, "Scroll Selector") != 0
        || tinyui_label_set_text(
               hint, "Swipe to switch between Wi-Fi, Bluetooth and Display.") != 0
        || tinyui_scroll_selector_add_item(scroll_selector, "wifi", "Wi-Fi") != 0
        || tinyui_scroll_selector_add_item(scroll_selector, "bluetooth", "Bluetooth") != 0
        || tinyui_scroll_selector_add_item(scroll_selector, "display", "Display") != 0
        || tinyui_scroll_selector_add_item(scroll_selector, "sound", "Sound") != 0
        || tinyui_scroll_selector_add_item(scroll_selector, "privacy", "Privacy") != 0
        || tinyui_scroll_selector_set_selected_index(scroll_selector, 1) != 0) {
        return TINYUI_ERROR_BACKEND;
    }

    return TINYUI_OK;
}
