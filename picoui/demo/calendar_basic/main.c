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

static void make_ui(struct picoui_window *win)
{
    struct picoui_label *title;
    struct picoui_calendar *calendar;
    static const int cols[] = {320, 0};
    static const int rows[] = {28, 196, 0};

    picoui_grid_set_columns(win, cols, 2);
    picoui_grid_set_rows(win, rows, 3);
    picoui_grid_set_gap(win, 12, 12);
    picoui_grid_set_align(win, PICOUI_ALIGN_START, PICOUI_ALIGN_START);
    picoui_window_set_padding_group(win, 24, 24, 24, 24);

    title = picoui_label_create(win, "title");
    if (title != 0) {
        picoui_label_set_text(title, "Calendar");
        picoui_widget_set_size((struct picoui_widget *)title, 220, 28);
        picoui_widget_set_grid_cell((struct picoui_widget *)title,
                                    0, 0, 1, 1,
                                    PICOUI_ALIGN_START,
                                    PICOUI_ALIGN_CENTER);
    }

    calendar = picoui_calendar_create_with_props(
        win,
        &(struct picoui_calendar_props){
            .id = "calendar",
            .year = 2026,
            .month = 6,
            .day = 15,
            .width = 280,
            .height = 180,
            .show_header = 1,
            .header_format = "yyyy/mm/dd",
        });
    if (calendar != 0) {
        picoui_widget_set_grid_cell((struct picoui_widget *)calendar,
                                    0, 1, 1, 1,
                                    PICOUI_ALIGN_START,
                                    PICOUI_ALIGN_CENTER);
    }
}

static int create_demo_ui(void)
{
    struct picoui_screen *screen = picoui_screen_active();
    struct picoui_window *win;

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
    if (create_demo_ui() != 0 || g_root_window == 0) {
        picoui_deinit();
        return 1;
    }

    while (1) {
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
