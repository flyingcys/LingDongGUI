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

static void make_ui(struct picoui_window *win)
{
    struct picoui_label *title;
    struct picoui_arc_props props = {
        .id = "arc",
        .bg_start_angle = 30.0f,
        .bg_end_angle = 300.0f,
        .fg_end_angle = 45.0f,
        .rotation_angle = 12.0f,
        .bg_color = 0xCCD5E3,
        .fg_color = 0x2B6CB0,
    };
    struct picoui_arc *arc;

    picoui_grid_set_columns(win, (const int[]){240, 0}, 2);
    picoui_grid_set_rows(win, (const int[]){28, 180, 0}, 3);
    picoui_grid_set_gap(win, 16, 16);
    picoui_grid_set_align(win, PICOUI_ALIGN_CENTER, PICOUI_ALIGN_CENTER);
    picoui_window_set_padding_group(win, 24, 24, 24, 24);

    title = picoui_label_create(win, "title");
    arc = picoui_arc_create_with_props((struct picoui_widget *)win, &props);

    picoui_label_set_text(title, "Arc");
    picoui_widget_set_size((struct picoui_widget *)title, 220, 28);
    picoui_widget_set_size((struct picoui_widget *)arc, 160, 160);
    picoui_widget_set_grid_cell((struct picoui_widget *)title, 0, 0, 1, 1, PICOUI_ALIGN_CENTER, PICOUI_ALIGN_CENTER);
    picoui_widget_set_grid_cell((struct picoui_widget *)arc, 0, 1, 1, 1, PICOUI_ALIGN_CENTER, PICOUI_ALIGN_CENTER);
}

void tinyui_demo_arc_basic(void)
{
    tinyui_obj_t *screen = tinyui_screen_create();
    struct picoui_window *win = (struct picoui_window *)screen;

    if (win == 0) {
        return;
    }

    make_ui(win);
    tinyui_screen_load(screen);
}
