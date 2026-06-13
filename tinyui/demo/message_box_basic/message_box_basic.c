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

#include "message_box_basic/message_box_basic.h"
#include "tinyui.h"

static void make_ui(struct picoui_window *win)
{
    struct picoui_label *title = picoui_label_create(win, "title");
    struct picoui_message_box *message_box;
    struct picoui_message_box_props props = {
        .id = "message_box",
        .title = "Update",
        .message = "Apply settings?",
        .confirm_text = "OK",
    };

    picoui_label_set_text(title, "Message Box");
    message_box = picoui_message_box_create_with_props((struct picoui_widget *)win, &props);
    if (message_box != 0) {
        (void)picoui_widget_set_pos((struct picoui_widget *)message_box, 110, 180);
    }
}

void tinyui_demo_message_box_basic(void)
{
    tinyui_obj_t *screen = tinyui_screen_create();
    struct picoui_window *win = (struct picoui_window *)screen;
    if (win == 0) return;
    make_ui(win);
    tinyui_screen_load(screen);
}
