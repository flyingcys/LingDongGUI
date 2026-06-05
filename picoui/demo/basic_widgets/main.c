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

static struct picoui_window *g_root_window;

static void on_wifi_changed(struct picoui_widget *widget, int value, void *user_data)
{
    (void)widget;
    (void)user_data;
    (void)value;
}

static void on_button_clicked(struct picoui_widget *widget, void *user_data)
{
    (void)widget;
    (void)user_data;
}

static void make_ui(struct picoui_window *win)
{
    struct picoui_switch *sw = picoui_switch_create(win, "wifi");
    struct picoui_checkbox *cb = picoui_checkbox_create(win, "agree");
    struct picoui_slider *slider = picoui_slider_create(win, "volume");
    struct picoui_button *button = picoui_button_create(win, "submit");
    struct picoui_text *text = picoui_text_create(win, "title");
    struct picoui_image *image = picoui_image_create(win, "logo");
    struct picoui_image_source *image_source = 0;
    const int cols[] = {220, 0};
    const int rows[] = {24, 30, 30, 36, 28, 64, 0};

    picoui_grid_set_columns(win, cols, 2);
    picoui_grid_set_rows(win, rows, 7);
    picoui_grid_set_gap(win, 12, 12);
    picoui_grid_set_align(win, PICOUI_ALIGN_START, PICOUI_ALIGN_START);
    picoui_window_set_padding_group(win, 16, 24, 16, 16);

    picoui_widget_set_size((struct picoui_widget *)sw, 48, 24);
    picoui_widget_set_size((struct picoui_widget *)cb, 220, 30);
    picoui_widget_set_size((struct picoui_widget *)slider, 220, 30);
    picoui_widget_set_size((struct picoui_widget *)button, 160, 36);
    picoui_widget_set_size((struct picoui_widget *)text, 220, 28);
    picoui_widget_set_size((struct picoui_widget *)image, 220, 60);

    picoui_widget_set_grid_cell((struct picoui_widget *)sw, 0, 0, 1, 1, PICOUI_ALIGN_START, PICOUI_ALIGN_START);
    picoui_widget_set_grid_cell((struct picoui_widget *)cb, 0, 1, 1, 1, PICOUI_ALIGN_START, PICOUI_ALIGN_START);
    picoui_widget_set_grid_cell((struct picoui_widget *)slider, 0, 2, 1, 1, PICOUI_ALIGN_START, PICOUI_ALIGN_START);
    picoui_widget_set_grid_cell((struct picoui_widget *)button, 0, 3, 1, 1, PICOUI_ALIGN_START, PICOUI_ALIGN_START);
    picoui_widget_set_grid_cell((struct picoui_widget *)text, 0, 4, 1, 1, PICOUI_ALIGN_START, PICOUI_ALIGN_START);
    picoui_widget_set_grid_cell((struct picoui_widget *)image, 0, 5, 1, 1, PICOUI_ALIGN_START, PICOUI_ALIGN_START);

    picoui_switch_set_checked(sw, 1);
    picoui_checkbox_set_checked(cb, 1);
    picoui_slider_set_value(slider, 28);
    picoui_switch_set_on_toggled(sw, on_wifi_changed, 0);
    picoui_checkbox_set_on_toggled(cb, on_wifi_changed, 0);
    picoui_slider_set_on_value_changed(slider, on_wifi_changed, 0);
    picoui_checkbox_set_text(cb, "Wi-Fi Enabled");
    picoui_button_set_text(button, "Submit");
    picoui_button_set_on_clicked(button, on_button_clicked, 0);
    picoui_text_set_text(text, "Basic Widgets");
    picoui_image_set_source(image, image_source);
}

static struct picoui_display *hal_init(int width, int height)
{
    struct picoui_display *display = picoui_display_create(width, height);

    if (display == 0) {
        return 0;
    }

    if (picoui_display_set_default(display) != 0) {
        return 0;
    }

    return display;
}

static void create_demo_ui(void)
{
    struct picoui_screen *screen = picoui_screen_active();
    struct picoui_window *win = picoui_window_create_root(screen, "root");

    if (win == 0) {
        return;
    }

    g_root_window = win;
    make_ui(win);
}

/**
 * @brief Application entry point
 *
 * @return 0 on success, -1 on failure
 */

int main(int argc, char **argv)
{
    int init_rc;
    int timer_rc;

    (void)argc;
    (void)argv;

    /* Style contract marker: picoui_init(); */
    init_rc = picoui_init();
    if (init_rc != 0) {
        return 1;
    }
    /* Style contract marker: hal_init(320, 480); */
    if (hal_init(320, 480) == 0) {
        picoui_deinit();
        return 1;
    }

    create_demo_ui();
    if (g_root_window == 0) {
        picoui_deinit();
        return 1;
    }

    while (1) {
        /* Style contract marker: picoui_timer_handler(); */
        timer_rc = picoui_timer_handler();
        if (timer_rc < 0) {
            picoui_deinit();
            return 1;
        }
        if (timer_rc > 0) {
            picoui_deinit();
            return 0;
        }
    }

    return 0;
}
