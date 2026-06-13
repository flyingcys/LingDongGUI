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
#include "image.h"

extern const unsigned char c_tileQuaterArcGRAY8;

static struct tinyui_image_source s_animation_source = {
    .img_tile = (void *)&c_tileQuaterArcGRAY8,
    .mask_tile = 0,
};

static void make_ui(struct tinyui_window *win)
{
    struct tinyui_label *title;
    struct tinyui_animation *animation;
    struct tinyui_animation_props props = {
        .id = "animation",
        .width = 62,
        .height = 31,
        .period_ms = 120,
        .source = &s_animation_source,
    };

    title = tinyui_label_create(win, "title");
    animation = tinyui_animation_create_with_props((struct tinyui_widget *)win, &props);

    tinyui_label_set_text(title, "Animation");
    tinyui_widget_set_pos((struct tinyui_widget *)title, 32, 32);
    tinyui_widget_set_pos((struct tinyui_widget *)animation, 160, 120);
    (void)tinyui_animation_show_frame(animation, 0);
}

static int run_demo(void)
{
    struct tinyui_app *app = tinyui_app_create();
    struct tinyui_window *win;

    if (app == 0) {
        return 1;
    }

    win = tinyui_window_create(app, "root");
    if (win == 0) {
        tinyui_app_destroy(app);
        return 1;
    }

    make_ui(win);
    if (tinyui_app_run(app, win) != 0) {
        tinyui_app_destroy(app);
        return 1;
    }
    tinyui_app_destroy(app);
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
