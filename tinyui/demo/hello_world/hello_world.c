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

#include "hello_world/hello_world.h"
#include "tinyui.h"

tinyui_result_t tinyui_demo_hello_world_build(tinyui_obj_t *screen)
{
    tinyui_obj_t *label;
    tinyui_obj_t *button;
    tinyui_result_t result;

    if (screen == NULL) {
        return TINYUI_ERROR_INVALID_ARG;
    }

    result = tinyui_flex_set_flow(screen, TINYUI_FLEX_FLOW_COLUMN);
    if (result != TINYUI_OK) {
        return result;
    }
    result = tinyui_flex_set_align(screen,
                                   TINYUI_ALIGN_CENTER,
                                   TINYUI_ALIGN_CENTER,
                                   TINYUI_ALIGN_CENTER);
    if (result != TINYUI_OK) {
        return result;
    }
    result = tinyui_flex_set_gap(screen, 12, 12);
    if (result != TINYUI_OK) {
        return result;
    }

    label = tinyui_label_create(screen);
    button = tinyui_button_create(screen);
    if (label == NULL || button == NULL) {
        return TINYUI_ERROR_NO_MEMORY;
    }

    if (tinyui_label_set_text(label, "Hello TINYUI") != 0) {
        return TINYUI_ERROR_BACKEND;
    }
    if (tinyui_button_set_text(button, "OK") != 0) {
        return TINYUI_ERROR_BACKEND;
    }

    return TINYUI_OK;
}
