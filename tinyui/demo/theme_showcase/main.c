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

static int run_demo(void)
{
    struct tinyui_theme *theme = tinyui_theme_create();
    struct tinyui_app *app = tinyui_app_create();
    struct tinyui_window *win;

    if (theme == 0 || app == 0) {
        tinyui_theme_destroy(theme);
        tinyui_app_destroy(app);
        return 1;
    }

    if (tinyui_app_set_theme(app, theme) != 0) {
        tinyui_theme_destroy(theme);
        tinyui_app_destroy(app);
        return 1;
    }

    win = tinyui_window_create(app, "root");
    if (win == 0) {
        tinyui_theme_destroy(theme);
        tinyui_app_destroy(app);
        return 1;
    }

    make_ui(win);
    if (tinyui_app_run(app, win) != 0) {
        tinyui_app_destroy(app);
        tinyui_theme_destroy(theme);
        return 1;
    }
    tinyui_app_destroy(app);
    tinyui_theme_destroy(theme);
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
