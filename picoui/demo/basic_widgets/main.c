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

#include "picoui/core.h"
#include "picoui/screen.h"
#include "picoui/window.h"
#include "picoui/label.h"
#include "picoui/button.h"
#include "picoui/port/sdl_v1_1.h"

static struct picoui_window *g_root_window;
static struct picoui_label *g_status_label;

static void on_button_clicked(struct picoui_widget *widget, void *user_data)
{
    (void)widget;
    (void)user_data;

    if (g_status_label != 0) {
        (void)picoui_label_set_text(g_status_label, "button clicked");
    }
}

static int make_ui(struct picoui_window *win)
{
    struct picoui_label *title;
    struct picoui_button *button;

    title = picoui_label_create(win, "title");
    if (title == 0) {
        return -1;
    }
    if (picoui_label_set_text(title, "v1.1 basic widgets") != 0) {
        return -1;
    }

    g_status_label = picoui_label_create(win, "status");
    if (g_status_label == 0) {
        return -1;
    }
    if (picoui_label_set_text(g_status_label, "window label button") != 0) {
        return -1;
    }

    button = picoui_button_create(win, "action");
    if (button == 0) {
        return -1;
    }
    if (picoui_button_set_on_clicked(button, on_button_clicked, 0) != 0) {
        return -1;
    }

    return 0;
}

static int create_demo_ui(void)
{
    struct picoui_screen *screen;
    struct picoui_window *win;

    screen = picoui_screen_active();
    win = picoui_window_create_root(screen, "root");
    if (win == 0) {
        return -1;
    }

    g_root_window = win;
    if (make_ui(win) != 0) {
        g_root_window = 0;
        g_status_label = 0;
        return -1;
    }
    if (picoui_screen_load(screen) != 0) {
        g_root_window = 0;
        g_status_label = 0;
        return -1;
    }

    return 0;
}

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
    /* Style contract marker: picoui_sdl_hal_init(320, 480); */
    if (picoui_sdl_hal_init(320, 480) != 0) {
        picoui_deinit();
        return 1;
    }

    if (create_demo_ui() != 0 || g_root_window == 0) {
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
