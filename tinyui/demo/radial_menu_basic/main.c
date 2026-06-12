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

static void make_ui(struct picoui_window *win)
{
    struct picoui_label *title;
    struct picoui_radial_menu *radial_menu;

    picoui_grid_set_columns(win, (const int[]){240, 0}, 2);
    picoui_grid_set_rows(win, (const int[]){28, 140, 0}, 3);
    picoui_grid_set_gap(win, 16, 16);
    picoui_grid_set_align(win, PICOUI_ALIGN_CENTER, PICOUI_ALIGN_CENTER);
    picoui_window_set_padding_group(win, 24, 24, 24, 24);

    title = picoui_label_create(win, "title");
    radial_menu = picoui_radial_menu_create((struct picoui_widget *)win, "radial_menu");

    picoui_label_set_text(title, "Radial Menu");
    picoui_widget_set_size((struct picoui_widget *)title, 220, 28);
    picoui_widget_set_size((struct picoui_widget *)radial_menu, 194, 96);
    picoui_radial_menu_add_item(radial_menu, "weather");
    picoui_radial_menu_add_item(radial_menu, "note");
    picoui_radial_menu_add_item(radial_menu, "book");
    picoui_radial_menu_add_item(radial_menu, "chart");
    picoui_radial_menu_set_selected_index(radial_menu, 1);
    picoui_widget_set_grid_cell((struct picoui_widget *)title, 0, 0, 1, 1, PICOUI_ALIGN_CENTER, PICOUI_ALIGN_CENTER);
    picoui_widget_set_grid_cell((struct picoui_widget *)radial_menu, 0, 1, 1, 1, PICOUI_ALIGN_CENTER, PICOUI_ALIGN_CENTER);
}

static int run_demo(void)
{
    struct picoui_theme *theme = picoui_theme_create();
    struct picoui_app *app = picoui_app_create();
    struct picoui_window *win;

    if (theme == 0 || app == 0) {
        picoui_theme_destroy(theme);
        picoui_app_destroy(app);
        return 1;
    }
    if (picoui_app_set_theme(app, theme) != 0) {
        picoui_theme_destroy(theme);
        picoui_app_destroy(app);
        return 1;
    }
    win = picoui_window_create(app, "root");
    if (win == 0) {
        picoui_theme_destroy(theme);
        picoui_app_destroy(app);
        return 1;
    }
    make_ui(win);
    if (picoui_app_run(app, win) != 0) {
        picoui_app_destroy(app);
        picoui_theme_destroy(theme);
        return 1;
    }
    picoui_app_destroy(app);
    picoui_theme_destroy(theme);
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
