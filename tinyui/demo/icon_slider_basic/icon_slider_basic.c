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

#include "icon_slider_basic/icon_slider_basic.h"
#include "tinyui.h"

#include <stdio.h>

static tinyui_image_source_t s_icons[5];

static void on_icon_selected(tinyui_obj_t *icon_slider, int index, void *user_data)
{
    (void)icon_slider;
    (void)user_data;
    printf("TINYUI_EVENT_TRACE_LINE=icon_slider:SELECTED:%d\n", index);
    fflush(stdout);
}

tinyui_result_t tinyui_demo_icon_slider_basic_build(tinyui_obj_t *screen)
{
    tinyui_obj_t *title;
    tinyui_obj_t *icon_slider;
    static const tinyui_builtin_image_t builtins[5] = {
        TINYUI_BUILTIN_IMAGE_WEATHER,
        TINYUI_BUILTIN_IMAGE_NOTE,
        TINYUI_BUILTIN_IMAGE_BOOK,
        TINYUI_BUILTIN_IMAGE_CHART,
        TINYUI_BUILTIN_IMAGE_LETTER_PAPER,
    };
    static const char *const ids[5] = {"weather", "note", "book", "chart", "paper"};
    static const char *const labels[5] = {"Weather", "Note", "Book", "Chart", "Paper"};
    int i;

    if (screen == NULL) {
        return TINYUI_ERROR_INVALID_ARG;
    }

    (void)tinyui_obj_set_bg_color(screen, 0xF6F8FAU);

    for (i = 0; i < 5; ++i) {
        if (tinyui_image_source_from_builtin(builtins[i], &s_icons[i]) != TINYUI_OK) {
            return TINYUI_ERROR_BACKEND;
        }
    }

    title = tinyui_label_create(screen);
    icon_slider = tinyui_icon_slider_create(screen);
    if (title == NULL || icon_slider == NULL) {
        return TINYUI_ERROR_NO_MEMORY;
    }

    if (tinyui_obj_set_pos(title, 32, 24) != TINYUI_OK
        || tinyui_obj_set_size(title, 240, 28) != TINYUI_OK
        || tinyui_label_set_text(title, "Icon Slider") != 0
        || tinyui_label_set_text_color(title, 0x102030U) != 0) {
        return TINYUI_ERROR_BACKEND;
    }

    if (tinyui_obj_set_pos(icon_slider, 24, 100) != TINYUI_OK
        || tinyui_obj_set_size(icon_slider, 432, 86) != TINYUI_OK) {
        return TINYUI_ERROR_BACKEND;
    }

    for (i = 0; i < 5; ++i) {
        if (tinyui_icon_slider_add_item_with_source(icon_slider,
                                                   ids[i],
                                                   labels[i],
                                                   &s_icons[i]) != 0) {
            return TINYUI_ERROR_BACKEND;
        }
    }
    if (tinyui_icon_slider_set_selected_index(icon_slider, 1) != 0) {
        return TINYUI_ERROR_BACKEND;
    }
    tinyui_icon_slider_set_on_selected(icon_slider, on_icon_selected, NULL);

    printf("TINYUI_SCENARIO=icon_slider_basic\n");
    fflush(stdout);
    return TINYUI_OK;
}
