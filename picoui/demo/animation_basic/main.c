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
#include "picoui/image.h"

extern const unsigned char c_tileQuaterArcGRAY8;

static struct picoui_image_source s_animation_source = {
    .img_tile = (void *)&c_tileQuaterArcGRAY8,
    .mask_tile = 0,
};

static void make_ui(struct picoui_window *win)
{
    struct picoui_label *title;
    struct picoui_animation *animation;
    struct picoui_animation_props props = {
        .id = "animation",
        .width = 62,
        .height = 31,
        .period_ms = 120,
        .source = &s_animation_source,
    };

    title = picoui_label_create(win, "title");
    animation = picoui_animation_create_with_props((struct picoui_widget *)win, &props);

    picoui_label_set_text(title, "Animation");
    picoui_widget_set_pos((struct picoui_widget *)title, 32, 32);
    picoui_widget_set_pos((struct picoui_widget *)animation, 160, 120);
    (void)picoui_animation_show_frame(animation, 0);
}

static int run_demo(void)
{
    struct picoui_app *app = picoui_app_create();
    struct picoui_window *win;

    if (app == 0) {
        return 1;
    }

    win = picoui_window_create(app, "root");
    if (win == 0) {
        picoui_app_destroy(app);
        return 1;
    }

    make_ui(win);
    if (picoui_app_run(app, win) != 0) {
        picoui_app_destroy(app);
        return 1;
    }
    picoui_app_destroy(app);
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
