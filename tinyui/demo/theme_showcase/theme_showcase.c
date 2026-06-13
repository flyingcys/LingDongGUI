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

#include "theme_showcase/theme_showcase.h"
#include "tinyui.h"

static void make_ui(struct tinyui_window *win)
{
    struct tinyui_label *title = tinyui_label_create(win, "title");
    struct tinyui_text *body = tinyui_text_create(win, "body");
    struct tinyui_button *accent = tinyui_button_create(win, "accent");

    tinyui_flex_set_flow(win, TINYUI_FLEX_FLOW_COLUMN);
    tinyui_flex_set_align(win,
                          TINYUI_ALIGN_CENTER,
                          TINYUI_ALIGN_CENTER,
                          TINYUI_ALIGN_CENTER);
    tinyui_flex_set_gap(win, 12, 12);

    tinyui_label_set_text(title, "Theme");
    tinyui_text_set_text(body, "Accent preview");
    tinyui_button_set_text(accent, "Primary");
}

void tinyui_demo_theme_showcase(void)
{
    tinyui_obj_t *screen = tinyui_screen_create();
    struct tinyui_window *win = (struct tinyui_window *)screen;
    if (win == 0) return;
    make_ui(win);
    tinyui_screen_load(screen);
}
