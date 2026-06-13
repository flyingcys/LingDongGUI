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
#include "button.h"
#include "label.h"
#include "layout.h"
#include "slider.h"
#include "switch.h"
#include "widget.h"
#include "window.h"

static int make_ui(struct picoui_window *win)
{
    struct picoui_label *title;
    struct picoui_switch *wifi;
    struct picoui_slider *brightness;
    struct picoui_button *apply;
    static const int cols[] = {-2, -1, 0};
    static const int rows[] = {32, 36, 40, 0};

    picoui_grid_set_columns(win, cols, 3);
    picoui_grid_set_rows(win, rows, 4);
    picoui_grid_set_gap(win, 12, 12);
    picoui_grid_set_align(win, PICOUI_ALIGN_STRETCH, PICOUI_ALIGN_START);

    title = picoui_label_create(win, "title");
    wifi = picoui_switch_create(win, "wifi");
    brightness = picoui_slider_create(win, "brightness");
    apply = picoui_button_create(win, "apply");

    if (title == 0 || wifi == 0 || brightness == 0 || apply == 0) {
        return -1;
    }

    picoui_label_set_text(title, "Settings");
    picoui_widget_set_size((struct picoui_widget *)wifi, 48, 24);
    picoui_switch_set_checked(wifi, 1);
    picoui_slider_set_value(brightness, 75);
    picoui_button_set_text(apply, "Apply");
    picoui_widget_set_grid_cell((struct picoui_widget *)title,
                                0, 0, 2, 1,
                                PICOUI_ALIGN_START,
                                PICOUI_ALIGN_CENTER);
    picoui_widget_set_grid_cell((struct picoui_widget *)wifi,
                                0, 1, 2, 1,
                                PICOUI_ALIGN_STRETCH,
                                PICOUI_ALIGN_CENTER);
    picoui_widget_set_grid_cell((struct picoui_widget *)brightness,
                                0, 2, 2, 1,
                                PICOUI_ALIGN_STRETCH,
                                PICOUI_ALIGN_CENTER);
    picoui_widget_set_grid_cell((struct picoui_widget *)apply,
                                1, 3, 1, 1,
                                PICOUI_ALIGN_END,
                                PICOUI_ALIGN_CENTER);

    return 0;
}

int tinyui_demo_settings_panel_build(tinyui_obj_t *screen)
{
    struct picoui_window *win = (struct picoui_window *)screen;

    if (screen == 0) {
        return -1;
    }

    return make_ui(win);
}