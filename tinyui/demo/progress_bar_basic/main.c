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

#include "picoui/picoui.h"

static void make_ui(struct picoui_window *win)
{
    struct picoui_label *title;
    struct picoui_progress_bar *primary;
    struct picoui_progress_bar *secondary;
    static const int cols[] = {80, 320, 0};
    static const int rows[] = {28, 24, 144, 0};

    picoui_grid_set_columns(win, cols, 3);
    picoui_grid_set_rows(win, rows, 4);
    picoui_grid_set_gap(win, 16, 16);
    picoui_grid_set_align(win, PICOUI_ALIGN_START, PICOUI_ALIGN_START);
    picoui_window_set_padding_group(win, 24, 24, 24, 24);

    title = picoui_label_create(win, "title");
    primary = picoui_progress_bar_create(win, "primary");
    secondary = picoui_progress_bar_create(win, "secondary");

    picoui_label_set_text(title, "Progress Bar");
    picoui_widget_set_size((struct picoui_widget *)title, 220, 28);
    picoui_widget_set_size((struct picoui_widget *)primary, 320, 24);
    picoui_widget_set_size((struct picoui_widget *)secondary, 48, 144);

    picoui_progress_bar_set_percent(primary, 72);
    picoui_progress_bar_set_percent(secondary, 40);
    picoui_progress_bar_set_horizontal(primary, 1);
    picoui_progress_bar_set_horizontal(secondary, 0);

    picoui_widget_set_grid_cell((struct picoui_widget *)title,
                                0, 0, 2, 1,
                                PICOUI_ALIGN_START,
                                PICOUI_ALIGN_CENTER);
    picoui_widget_set_grid_cell((struct picoui_widget *)primary,
                                1, 1, 1, 1,
                                PICOUI_ALIGN_START,
                                PICOUI_ALIGN_CENTER);
    picoui_widget_set_grid_cell((struct picoui_widget *)secondary,
                                0, 2, 1, 1,
                                PICOUI_ALIGN_CENTER,
                                PICOUI_ALIGN_START);
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
