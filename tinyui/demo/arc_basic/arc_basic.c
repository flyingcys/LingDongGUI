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

#include "arc_basic/arc_basic.h"
#include "tinyui.h"
static void make_ui(struct tinyui_window *win)
{
    struct tinyui_label *title;
    struct tinyui_arc_props props = {
        .id = "arc",
        .bg_start_angle = 30.0f,
        .bg_end_angle = 300.0f,
        .fg_end_angle = 45.0f,
        .rotation_angle = 12.0f,
        .bg_color = 0xCCD5E3,
        .fg_color = 0x2B6CB0,
    };
    struct tinyui_arc *arc;

    tinyui_grid_set_columns(win, (const int[]){240, 0}, 2);
    tinyui_grid_set_rows(win, (const int[]){28, 180, 0}, 3);
    tinyui_grid_set_gap(win, 16, 16);
    tinyui_grid_set_align(win, TINYUI_ALIGN_CENTER, TINYUI_ALIGN_CENTER);
    tinyui_window_set_padding(win, 24, 24, 24, 24);

    title = tinyui_label_create(win, "title");
    arc = tinyui_arc_create_with_props((struct tinyui_widget *)win, &props);

    tinyui_label_set_text(title, "Arc");
    tinyui_widget_set_size((struct tinyui_widget *)title, 220, 28);
    tinyui_widget_set_size((struct tinyui_widget *)arc, 160, 160);
    tinyui_widget_set_grid_cell((struct tinyui_widget *)title, 0, 0, 1, 1, TINYUI_ALIGN_CENTER, TINYUI_ALIGN_CENTER);
    tinyui_widget_set_grid_cell((struct tinyui_widget *)arc, 0, 1, 1, 1, TINYUI_ALIGN_CENTER, TINYUI_ALIGN_CENTER);
}

void tinyui_demo_arc_basic(void)
{
    tinyui_obj_t *screen = tinyui_screen_create();
    struct tinyui_window *win = (struct tinyui_window *)screen;

    if (win == 0) {
        return;
    }

    make_ui(win);
    tinyui_screen_load(screen);
}
