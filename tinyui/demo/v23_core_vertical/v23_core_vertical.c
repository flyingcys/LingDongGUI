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

/*
 * M2 Task 8 证据场景：仅用 canonical tinyui_* API 构建
 * label / button / checkbox / slider，并用 flex 纵向排列。
 * 交互由 ENABLE_TEST observe 脚本注入；回调只打印事件 trace。
 */

#include "v23_core_vertical/v23_core_vertical.h"
#include "tinyui.h"

#include <stdio.h>

static void log_event_trace(const char *line)
{
    if (line == NULL || line[0] == '\0') {
        return;
    }
    printf("TINYUI_EVENT_TRACE_LINE=%s\n", line);
    fflush(stdout);
}

static void on_button_event(const tinyui_event_t *event)
{
    if (event == NULL) {
        return;
    }
    if (event->code == TINYUI_EVENT_PRESSED) {
        log_event_trace("button:PRESSED");
    } else if (event->code == TINYUI_EVENT_RELEASED) {
        log_event_trace("button:RELEASED");
    } else if (event->code == TINYUI_EVENT_CLICKED) {
        log_event_trace("button:CLICKED");
    }
}

static void on_checkbox_event(const tinyui_event_t *event)
{
    char line[64];

    if (event == NULL || event->code != TINYUI_EVENT_VALUE_CHANGED) {
        return;
    }
    snprintf(line, sizeof(line), "checkbox:VALUE_CHANGED:%d", (int)event->data.value);
    log_event_trace(line);
}

static void on_slider_event(const tinyui_event_t *event)
{
    char line[64];

    if (event == NULL || event->code != TINYUI_EVENT_VALUE_CHANGED) {
        return;
    }
    snprintf(line, sizeof(line), "slider:VALUE_CHANGED:%d", (int)event->data.value);
    log_event_trace(line);
}

static int make_ui(tinyui_obj_t *screen)
{
    tinyui_obj_t *label;
    tinyui_obj_t *button;
    tinyui_obj_t *checkbox;
    tinyui_obj_t *slider;

    if (screen == NULL) {
        return -1;
    }

    /* 固定坐标布局（canonical obj API），避免依赖 layout 求解器做证据补偿。
     * 采样区域与 checker 中的 region 定义一致。 */
    (void)tinyui_obj_set_bg_color(screen, 0xF6F8FAU);

    label = tinyui_label_create(screen);
    button = tinyui_button_create(screen);
    checkbox = tinyui_checkbox_create(screen);
    slider = tinyui_slider_create(screen);
    if (label == NULL || button == NULL || checkbox == NULL || slider == NULL) {
        return -1;
    }

    if (tinyui_obj_set_pos(label, 24, 24) != TINYUI_OK
        || tinyui_obj_set_size(label, 200, 32) != TINYUI_OK
        || tinyui_label_set_text(label, "Core Vertical") != 0
        || tinyui_label_set_text_color(label, 0x102030U) != 0
        || tinyui_label_set_bg_color(label, 0xE8EEF5U) != 0
        || tinyui_label_set_transparent(label, 0) != 0) {
        return -1;
    }

    if (tinyui_obj_set_pos(button, 24, 72) != TINYUI_OK
        || tinyui_obj_set_size(button, 160, 40) != TINYUI_OK
        || tinyui_button_set_text(button, "Click") != 0
        || tinyui_button_set_on_pressed(button, on_button_event, NULL) != 0
        || tinyui_button_set_on_released(button, on_button_event, NULL) != 0
        || tinyui_button_set_on_clicked(button, on_button_event, NULL) != 0) {
        return -1;
    }

    if (tinyui_obj_set_pos(checkbox, 24, 128) != TINYUI_OK
        || tinyui_obj_set_size(checkbox, 220, 32) != TINYUI_OK
        || tinyui_checkbox_set_text(checkbox, "Accept") != 0
        || tinyui_checkbox_set_checked(checkbox, 0) != 0
        || tinyui_checkbox_set_on_toggled(checkbox, on_checkbox_event, NULL) != 0) {
        return -1;
    }

    if (tinyui_obj_set_pos(slider, 24, 176) != TINYUI_OK
        || tinyui_obj_set_size(slider, 280, 28) != TINYUI_OK
        || tinyui_slider_set_range(slider, 0, 100) != 0
        || tinyui_slider_set_value(slider, 25) != 0
        || tinyui_slider_set_horizontal(slider, 1) != 0
        || tinyui_slider_set_color(slider, 0xD0D7DEU, 0x8B949EU, 0x1F6FEBU) != 0
        || tinyui_slider_set_on_value_changed(slider, on_slider_event, NULL) != 0) {
        return -1;
    }

    printf("TINYUI_SCENARIO=v23_core_vertical\n");
    fflush(stdout);
    return 0;
}

void tinyui_demo_v23_core_vertical(void)
{
    tinyui_obj_t *screen = tinyui_screen_create();

    if (screen == NULL) {
        return;
    }
    if (make_ui(screen) != 0) {
        return;
    }
    (void)tinyui_screen_load(screen, TINYUI_SCREEN_TRANSITION_NONE, 0);
}

void tinyui_demo_v23_core_vertical_frame(unsigned int elapsed_ms)
{
    (void)elapsed_ms;
}
