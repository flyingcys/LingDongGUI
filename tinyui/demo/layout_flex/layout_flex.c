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

#include "layout_flex/layout_flex.h"
#include "tinyui.h"

tinyui_result_t tinyui_demo_layout_flex_build(tinyui_obj_t *screen)
{
    tinyui_obj_t *a;
    tinyui_obj_t *b;
    tinyui_obj_t *c;
    tinyui_result_t result;

    if (screen == NULL) {
        return TINYUI_ERROR_INVALID_ARG;
    }

    result = tinyui_flex_set_flow(screen, TINYUI_FLEX_FLOW_ROW_WRAP);
    if (result != TINYUI_OK) {
        return result;
    }
    result = tinyui_flex_set_align(screen,
                                   TINYUI_ALIGN_START,
                                   TINYUI_ALIGN_CENTER,
                                   TINYUI_ALIGN_SPACE_AROUND);
    if (result != TINYUI_OK) {
        return result;
    }
    result = tinyui_flex_set_gap(screen, 8, 12);
    if (result != TINYUI_OK) {
        return result;
    }

    a = tinyui_button_create(screen);
    b = tinyui_button_create(screen);
    c = tinyui_button_create(screen);
    if (a == NULL || b == NULL || c == NULL) {
        return TINYUI_ERROR_NO_MEMORY;
    }

    if (tinyui_button_set_text(a, "One") != 0
        || tinyui_button_set_text(b, "Two") != 0
        || tinyui_button_set_text(c, "Three") != 0) {
        return TINYUI_ERROR_BACKEND;
    }

    if (tinyui_obj_set_flex_grow(a, 1) != TINYUI_OK
        || tinyui_obj_set_flex_grow(b, 1) != TINYUI_OK
        || tinyui_obj_set_flex_new_track(c, 1) != TINYUI_OK) {
        return TINYUI_ERROR_BACKEND;
    }

    return TINYUI_OK;
}
