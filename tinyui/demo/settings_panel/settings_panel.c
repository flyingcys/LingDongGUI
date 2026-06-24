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

#include "settings_panel/settings_panel.h"
#include "tinyui.h"








static int make_ui(struct tinyui_window *win)
{
    struct tinyui_label *title;
    struct tinyui_switch *wifi;
    struct tinyui_slider *brightness;
    struct tinyui_button *apply;
    static const int cols[] = {-2, -1, 0};
    static const int rows[] = {32, 36, 40, 0};

    tinyui_grid_set_columns(win, cols, 3);
    tinyui_grid_set_rows(win, rows, 4);
    tinyui_grid_set_gap(win, 12, 12);
    tinyui_grid_set_align(win, TINYUI_ALIGN_STRETCH, TINYUI_ALIGN_START);

    title = tinyui_label_create(win, "title");
    wifi = tinyui_switch_create(win, "wifi");
    brightness = tinyui_slider_create(win, "brightness");
    apply = tinyui_button_create(win, "apply");

    if (title == 0 || wifi == 0 || brightness == 0 || apply == 0) {
        return -1;
    }

    tinyui_label_set_text(title, "Settings");
    tinyui_widget_set_size((struct tinyui_widget *)wifi, 48, 24);
    tinyui_switch_set_checked(wifi, 1);
    tinyui_slider_set_value(brightness, 75);
    tinyui_button_set_text(apply, "Apply");
    tinyui_widget_set_grid_cell((struct tinyui_widget *)title,
                                0, 0, 2, 1,
                                TINYUI_ALIGN_START,
                                TINYUI_ALIGN_CENTER);
    tinyui_widget_set_grid_cell((struct tinyui_widget *)wifi,
                                0, 1, 2, 1,
                                TINYUI_ALIGN_STRETCH,
                                TINYUI_ALIGN_CENTER);
    tinyui_widget_set_grid_cell((struct tinyui_widget *)brightness,
                                0, 2, 2, 1,
                                TINYUI_ALIGN_STRETCH,
                                TINYUI_ALIGN_CENTER);
    tinyui_widget_set_grid_cell((struct tinyui_widget *)apply,
                                1, 3, 1, 1,
                                TINYUI_ALIGN_END,
                                TINYUI_ALIGN_CENTER);

    return 0;
}

void tinyui_demo_settings_panel(void)
{
    tinyui_obj_t *screen = tinyui_screen_create();
    struct tinyui_window *win = (struct tinyui_window *)screen;
    if (win == 0) return;
    make_ui(win);
    tinyui_screen_load(screen);
}