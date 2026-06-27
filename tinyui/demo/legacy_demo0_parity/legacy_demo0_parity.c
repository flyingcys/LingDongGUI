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

#include "legacy_demo0_parity/legacy_demo0_parity.h"
#include "tinyui.h"

static const char *const g_legacy_demo0_truth_fields[] = {
    "button@10, 10:123",
    "\"123\"",
    "text@300, 10:123\\n12333",
    "\"123\\n12333\"",
    "switch@300, 226:OFF",
    "\"OFF\"",
    "switch_label@356, 218:OFF",
    "qrcode@500, 10:legacy token",
    "g_legacy_qrcode_truth",
    "\"l\" \"d\" \"g\" \"u\" \"i\"",
    "list@850, 280:1/10/123/99/7",
    "\"title\"",
    "message_box@200, 150:title/12345678abcdefg\\n99556",
    "\"12345678abcdefg\\n99556\"",
    "calendar@50, 340:yyyy - mm - dd",
    "\"yyyy - mm - dd\"",
};

static const char g_legacy_qrcode_truth[] = "l" "d" "g" "u" "i";

void tinyui_demo_legacy_demo0_parity(void)
{
    tinyui_obj_t *screen = tinyui_screen_create();
    (void)g_legacy_demo0_truth_fields;

    if (screen == 0) {
        return;
    }

    tinyui_screen_load(screen);
}
