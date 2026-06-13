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

#include "layout_flex/layout_flex.h"
#include "tinyui.h"

static void make_ui(struct tinyui_window *win)
{
    struct tinyui_button *a;
    struct tinyui_button *b;
    struct tinyui_button *c;

    tinyui_flex_set_flow(win, TINYUI_FLEX_FLOW_ROW_WRAP);
    tinyui_flex_set_align(win,
                          TINYUI_ALIGN_START,
                          TINYUI_ALIGN_CENTER,
                          TINYUI_ALIGN_SPACE_AROUND);
    tinyui_flex_set_gap(win, 8, 12);

    a = tinyui_button_create(win, "first");
    b = tinyui_button_create(win, "second");
    c = tinyui_button_create(win, "third");
    tinyui_button_set_text(a, "One");
    tinyui_button_set_text(b, "Two");
    tinyui_button_set_text(c, "Three");
    tinyui_widget_set_flex_grow((struct tinyui_widget *)a, 1);
    tinyui_widget_set_flex_grow((struct tinyui_widget *)b, 1);
    tinyui_widget_set_flex_new_track((struct tinyui_widget *)c, 1);
}

void tinyui_demo_layout_flex(void)
{
    tinyui_obj_t *screen = tinyui_screen_create();
    struct tinyui_window *win = (struct tinyui_window *)screen;
    if (win == 0) return;
    make_ui(win);
    tinyui_screen_load(screen);
}
