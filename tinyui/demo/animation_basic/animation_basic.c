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

#include "animation_basic/animation_basic.h"
#include "tinyui.h"

static struct tinyui_image_source s_animation_source;
static int s_animation_source_ready;

static int ensure_animation_source(void)
{
    if (s_animation_source_ready != 0) {
        return 0;
    }

    if (tinyui_image_source_from_builtin(TINYUI_BUILTIN_IMAGE_ARC_QUARTER,
                                         &s_animation_source) != 0) {
        return -1;
    }

    s_animation_source_ready = 1;
    return 0;
}

static void make_ui(struct tinyui_window *win)
{
    struct tinyui_label *title;
    struct tinyui_animation *animation;
    struct tinyui_animation_props props = {
        .id = "animation",
        .width = 25,
        .height = 25,
        .period_ms = 120,
        .source = &s_animation_source,
    };

    if (ensure_animation_source() != 0) {
        return;
    }

    title = tinyui_label_create(win, "title");
    animation = tinyui_animation_create_with_props((struct tinyui_widget *)win, &props);

    tinyui_label_set_text(title, "Animation");
    tinyui_widget_set_pos((struct tinyui_widget *)title, 32, 32);
    tinyui_widget_set_pos((struct tinyui_widget *)animation, 160, 120);
    (void)tinyui_animation_show_frame(animation, 0);
}

void tinyui_demo_animation_basic(void)
{
    tinyui_obj_t *screen = tinyui_screen_create();
    struct tinyui_window *win = (struct tinyui_window *)screen;

    if (win == 0) {
        return;
    }

    make_ui(win);
    tinyui_screen_load(screen);
}
