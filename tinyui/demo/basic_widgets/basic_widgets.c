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
#include "runtime.h"
#include "button.h"
#include "checkbox.h"
#include "image.h"
#include "layout.h"
#include "slider.h"
#include "switch.h"
#include "text.h"
#include "widget.h"
#include "window.h"
#include <stdio.h>

static void on_wifi_changed(struct tinyui_widget *widget, int value, void *user_data)
{
    (void)widget;
    (void)user_data;
    (void)value;
}

static void on_button_clicked(struct tinyui_widget *widget, void *user_data)
{
    (void)widget;
    (void)user_data;
}

static int make_ui(struct tinyui_window *win)
{
    struct tinyui_switch *sw = tinyui_switch_create(win, "wifi");
    struct tinyui_checkbox *cb = tinyui_checkbox_create(win, "agree");
    struct tinyui_slider *slider = tinyui_slider_create(win, "volume");
    struct tinyui_button *button = tinyui_button_create(win, "submit");
    struct tinyui_text *text = tinyui_text_create(win, "title");
    struct tinyui_image *image = tinyui_image_create(win, "logo");
    struct tinyui_image_source *image_source = 0;
    const int cols[] = {220, 0};
    const int rows[] = {24, 30, 30, 36, 28, 64, 0};

    if (sw == 0 || cb == 0 || slider == 0 || button == 0 || text == 0 || image == 0) {
        fprintf(stdout,
                "TINYUI_BASIC_WIDGETS_DEBUG make_ui_create_failed sw=%p cb=%p slider=%p button=%p text=%p image=%p\n",
                (void *)sw,
                (void *)cb,
                (void *)slider,
                (void *)button,
                (void *)text,
                (void *)image);
        fflush(stdout);
        return -1;
    }

    tinyui_grid_set_columns(win, cols, 2);
    tinyui_grid_set_rows(win, rows, 7);
    tinyui_grid_set_gap(win, 12, 12);
    tinyui_grid_set_align(win, TINYUI_ALIGN_START, TINYUI_ALIGN_START);
    tinyui_window_set_padding(win, 16, 24, 16, 16);

    tinyui_widget_set_size((struct tinyui_widget *)sw, 48, 24);
    tinyui_widget_set_size((struct tinyui_widget *)cb, 220, 30);
    tinyui_widget_set_size((struct tinyui_widget *)slider, 220, 30);
    tinyui_widget_set_size((struct tinyui_widget *)button, 160, 36);
    tinyui_widget_set_size((struct tinyui_widget *)text, 220, 28);
    tinyui_widget_set_size((struct tinyui_widget *)image, 220, 60);

    tinyui_widget_set_grid_cell((struct tinyui_widget *)sw, 0, 0, 1, 1, TINYUI_ALIGN_START, TINYUI_ALIGN_START);
    tinyui_widget_set_grid_cell((struct tinyui_widget *)cb, 0, 1, 1, 1, TINYUI_ALIGN_START, TINYUI_ALIGN_START);
    tinyui_widget_set_grid_cell((struct tinyui_widget *)slider, 0, 2, 1, 1, TINYUI_ALIGN_START, TINYUI_ALIGN_START);
    tinyui_widget_set_grid_cell((struct tinyui_widget *)button, 0, 3, 1, 1, TINYUI_ALIGN_START, TINYUI_ALIGN_START);
    tinyui_widget_set_grid_cell((struct tinyui_widget *)text, 0, 4, 1, 1, TINYUI_ALIGN_START, TINYUI_ALIGN_START);
    tinyui_widget_set_grid_cell((struct tinyui_widget *)image, 0, 5, 1, 1, TINYUI_ALIGN_START, TINYUI_ALIGN_START);

    tinyui_switch_set_checked(sw, 1);
    tinyui_checkbox_set_checked(cb, 1);
    tinyui_slider_set_value(slider, 28);
    tinyui_switch_set_on_toggled(sw, on_wifi_changed, 0);
    tinyui_checkbox_set_on_toggled(cb, on_wifi_changed, 0);
    tinyui_slider_set_on_value_changed(slider, on_wifi_changed, 0);
    tinyui_checkbox_set_text(cb, "Wi-Fi Enabled");
    tinyui_button_set_text(button, "Submit");
    tinyui_button_set_on_clicked(button, on_button_clicked, 0);
    tinyui_text_set_text(text, "Basic Widgets");
    tinyui_image_set_source(image, image_source);

    return 0;
}

void tinyui_demo_basic_widgets(void)
{
    tinyui_obj_t *screen = tinyui_screen_create();
    struct tinyui_window *win = (struct tinyui_window *)screen;

    if (win == 0) {
        return;
    }

    if (make_ui(win) != 0) {
        return;
    }

    tinyui_screen_load(screen);
}
