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
#include "picoui/port/sdl.h"

static struct picoui_window *g_root_window;
static struct picoui_theme *g_theme;

static void make_ui(struct picoui_window *win)
{
    struct picoui_label *title;
    struct picoui_icon_slider *icon_slider;

    picoui_grid_set_columns(win, (const int[]){240, 0}, 2);
    picoui_grid_set_rows(win, (const int[]){28, 140, 0}, 3);
    picoui_grid_set_gap(win, 16, 16);
    picoui_grid_set_align(win, PICOUI_ALIGN_CENTER, PICOUI_ALIGN_CENTER);
    picoui_window_set_padding_group(win, 24, 24, 24, 24);

    title = picoui_label_create(win, "title");
    icon_slider = picoui_icon_slider_create((struct picoui_widget *)win, "icon_slider");

    picoui_label_set_text(title, "Icon Slider");
    picoui_widget_set_size((struct picoui_widget *)title, 220, 28);
    picoui_widget_set_size((struct picoui_widget *)icon_slider, 240, 86);
    picoui_icon_slider_add_item(icon_slider, "weather", "Weather");
    picoui_icon_slider_add_item(icon_slider, "note", "Note");
    picoui_icon_slider_add_item(icon_slider, "book", "Book");
    picoui_icon_slider_add_item(icon_slider, "chart", "Chart");
    picoui_icon_slider_add_item(icon_slider, "clock", "Clock");
    picoui_icon_slider_set_selected_index(icon_slider, 1);
    picoui_widget_set_grid_cell((struct picoui_widget *)title, 0, 0, 1, 1, PICOUI_ALIGN_CENTER, PICOUI_ALIGN_CENTER);
    picoui_widget_set_grid_cell((struct picoui_widget *)icon_slider, 0, 1, 1, 1, PICOUI_ALIGN_CENTER, PICOUI_ALIGN_CENTER);

    if (g_theme != 0) {
        (void)picoui_theme_apply_to_widget(g_theme,
                                           (struct picoui_widget *)win,
                                           PICOUI_PART_MAIN,
                                           PICOUI_STATE_DEFAULT);
        (void)picoui_theme_apply_to_widget(g_theme,
                                           (struct picoui_widget *)title,
                                           PICOUI_PART_MAIN,
                                           PICOUI_STATE_DEFAULT);
        (void)picoui_theme_apply_to_widget(g_theme,
                                           (struct picoui_widget *)title,
                                           PICOUI_PART_TEXT,
                                           PICOUI_STATE_DEFAULT);
        (void)picoui_theme_apply_to_widget(g_theme,
                                           (struct picoui_widget *)icon_slider,
                                           PICOUI_PART_MAIN,
                                           PICOUI_STATE_DEFAULT);
    }
}

static int create_demo_ui(void)
{
    struct picoui_screen *screen = picoui_screen_active();
    struct picoui_window *win;

    if (g_theme == 0 || screen == 0) {
        return -1;
    }

    win = picoui_window_create_root(screen, "root");
    if (win == 0) {
        return -1;
    }

    g_root_window = win;
    make_ui(win);
    if (picoui_screen_load(screen) != 0) {
        g_root_window = 0;
        return -1;
    }

    return 0;
}

/**
 * @brief Application entry point
 *
 * @return 0 on success, -1 on failure
 */

int main(void)
{
    int init_rc;
    int timer_rc;

    init_rc = picoui_init();
    if (init_rc != 0) {
        return 1;
    }
    if (picoui_sdl_hal_init(320, 480) != 0) {
        picoui_deinit();
        return 1;
    }

    g_theme = picoui_theme_create();
    if (g_theme == 0) {
        picoui_deinit();
        return 1;
    }

    if (create_demo_ui() != 0 || g_root_window == 0) {
        picoui_theme_destroy(g_theme);
        g_theme = 0;
        picoui_deinit();
        return 1;
    }

    while (1) {
        timer_rc = picoui_timer_handler();
        if (timer_rc < 0) {
            picoui_theme_destroy(g_theme);
            g_theme = 0;
            picoui_deinit();
            return 1;
        }
        if (timer_rc > 0) {
            picoui_theme_destroy(g_theme);
            g_theme = 0;
            picoui_deinit();
            return 0;
        }
    }

    return 0;
}
