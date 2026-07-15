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

#include "basic_widgets/basic_widgets.h"
#include "tinyui.h"

static void on_wifi_changed(const tinyui_event_t *event)
{
    (void)event;
}

static void on_button_clicked(const tinyui_event_t *event)
{
    (void)event;
}

tinyui_result_t tinyui_demo_basic_widgets_build(tinyui_obj_t *screen)
{
    tinyui_obj_t *sw;
    tinyui_obj_t *cb;
    tinyui_obj_t *slider;
    tinyui_obj_t *button;
    tinyui_obj_t *text;
    tinyui_obj_t *image;
    tinyui_result_t result;

    if (screen == NULL) {
        return TINYUI_ERROR_INVALID_ARG;
    }

    /* 原 1 列 grid 纵向堆叠意图保留为 canonical flex column。 */
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
    if (tinyui_window_set_padding(screen, 16, 24, 16, 16) != 0) {
        return TINYUI_ERROR_BACKEND;
    }

    sw = tinyui_switch_create(screen);
    cb = tinyui_checkbox_create(screen);
    slider = tinyui_slider_create(screen);
    button = tinyui_button_create(screen);
    text = tinyui_text_create(screen);
    image = tinyui_image_create(screen);
    if (sw == NULL || cb == NULL || slider == NULL || button == NULL
        || text == NULL || image == NULL) {
        return TINYUI_ERROR_NO_MEMORY;
    }

    if (tinyui_switch_set_checked(sw, 1) != 0
        || tinyui_switch_set_on_toggled(sw, on_wifi_changed, NULL) != 0
        || tinyui_checkbox_set_checked(cb, 1) != 0
        || tinyui_checkbox_set_text(cb, "Wi-Fi Enabled") != 0
        || tinyui_checkbox_set_on_toggled(cb, on_wifi_changed, NULL) != 0
        || tinyui_slider_set_value(slider, 28) != 0
        || tinyui_slider_set_on_value_changed(slider, on_wifi_changed, NULL) != 0
        || tinyui_button_set_text(button, "Submit") != 0
        || tinyui_button_set_on_clicked(button, on_button_clicked, NULL) != 0
        || tinyui_text_set_text(text, "Basic Widgets") != 0
        || tinyui_image_set_source(image, NULL) != 0) {
        return TINYUI_ERROR_BACKEND;
    }

    return TINYUI_OK;
}
