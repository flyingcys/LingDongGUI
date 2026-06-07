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

static void apply_theme(struct picoui_theme *theme,
                        struct picoui_window *window,
                        struct picoui_label *title,
                        struct picoui_switch *wifi,
                        struct picoui_slider *brightness,
                        struct picoui_button *apply)
{
    if (theme == 0 || window == 0 || title == 0 || wifi == 0 || brightness == 0 || apply == 0) {
        return;
    }

    (void)picoui_theme_apply_to_widget(theme,
                                       (struct picoui_widget *)window,
                                       PICOUI_PART_MAIN,
                                       PICOUI_STATE_DEFAULT);
    (void)picoui_theme_apply_to_widget(theme,
                                       (struct picoui_widget *)title,
                                       PICOUI_PART_MAIN,
                                       PICOUI_STATE_DEFAULT);
    (void)picoui_theme_apply_to_widget(theme,
                                       (struct picoui_widget *)title,
                                       PICOUI_PART_TEXT,
                                       PICOUI_STATE_DEFAULT);
    (void)picoui_theme_apply_to_widget(theme,
                                       (struct picoui_widget *)wifi,
                                       PICOUI_PART_MAIN,
                                       PICOUI_STATE_DEFAULT);
    (void)picoui_theme_apply_to_widget(theme,
                                       (struct picoui_widget *)brightness,
                                       PICOUI_PART_MAIN,
                                       PICOUI_STATE_DEFAULT);
    (void)picoui_theme_apply_to_widget(theme,
                                       (struct picoui_widget *)apply,
                                       PICOUI_PART_MAIN,
                                       PICOUI_STATE_DEFAULT);
    (void)picoui_theme_apply_to_widget(theme,
                                       (struct picoui_widget *)apply,
                                       PICOUI_PART_TEXT,
                                       PICOUI_STATE_DEFAULT);
}

static void make_ui(struct picoui_window *win)
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
    picoui_window_set_padding_group(win, 24, 24, 24, 24);

    title = picoui_label_create(win, "title");
    wifi = picoui_switch_create(win, "wifi");
    brightness = picoui_slider_create(win, "brightness");
    apply = picoui_button_create(win, "apply");

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
    apply_theme(g_theme, win, title, wifi, brightness, apply);
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
