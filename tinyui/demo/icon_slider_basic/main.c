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

#include "tinyui.h"

static void make_ui(struct tinyui_window *win)
{
    struct tinyui_label *title;
    struct tinyui_icon_slider *icon_slider;

    tinyui_grid_set_columns(win, (const int[]){240, 0}, 2);
    tinyui_grid_set_rows(win, (const int[]){28, 140, 0}, 3);
    tinyui_grid_set_gap(win, 16, 16);
    tinyui_grid_set_align(win, TINYUI_ALIGN_CENTER, TINYUI_ALIGN_CENTER);
    tinyui_window_set_padding_group(win, 24, 24, 24, 24);

    title = tinyui_label_create(win, "title");
    icon_slider = tinyui_icon_slider_create((struct tinyui_widget *)win, "icon_slider");

    tinyui_label_set_text(title, "Icon Slider");
    tinyui_widget_set_size((struct tinyui_widget *)title, 220, 28);
    tinyui_widget_set_size((struct tinyui_widget *)icon_slider, 240, 86);
    tinyui_icon_slider_add_item(icon_slider, "weather", "Weather");
    tinyui_icon_slider_add_item(icon_slider, "note", "Note");
    tinyui_icon_slider_add_item(icon_slider, "book", "Book");
    tinyui_icon_slider_add_item(icon_slider, "chart", "Chart");
    tinyui_icon_slider_add_item(icon_slider, "clock", "Clock");
    tinyui_icon_slider_set_selected_index(icon_slider, 1);
    tinyui_widget_set_grid_cell((struct tinyui_widget *)title, 0, 0, 1, 1, TINYUI_ALIGN_CENTER, TINYUI_ALIGN_CENTER);
    tinyui_widget_set_grid_cell((struct tinyui_widget *)icon_slider, 0, 1, 1, 1, TINYUI_ALIGN_CENTER, TINYUI_ALIGN_CENTER);
}

static int run_demo(void)
{
    struct tinyui_theme *theme = tinyui_theme_create();
    struct tinyui_app *app = tinyui_app_create();
    struct tinyui_window *win;

    if (theme == 0 || app == 0) {
        tinyui_theme_destroy(theme);
        tinyui_app_destroy(app);
        return 1;
    }
    if (tinyui_app_set_theme(app, theme) != 0) {
        tinyui_theme_destroy(theme);
        tinyui_app_destroy(app);
        return 1;
    }
    win = tinyui_window_create(app, "root");
    if (win == 0) {
        tinyui_theme_destroy(theme);
        tinyui_app_destroy(app);
        return 1;
    }
    make_ui(win);
    if (tinyui_app_run(app, win) != 0) {
        tinyui_app_destroy(app);
        tinyui_theme_destroy(theme);
        return 1;
    }
    tinyui_app_destroy(app);
    tinyui_theme_destroy(theme);
    return 0;
}

/**
 * @brief Application entry point
 *
 * @return 0 on success, -1 on failure
 */

int main(void)
{
    return run_demo();
}
