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

#include "qrcode_basic/qrcode_basic.h"
#include "tinyui.h"

#include <stdio.h>

tinyui_result_t tinyui_demo_qrcode_basic_build(tinyui_obj_t *screen)
{
    tinyui_obj_t *title;
    tinyui_obj_t *qrcode;

    if (screen == NULL) {
        return TINYUI_ERROR_INVALID_ARG;
    }

    (void)tinyui_obj_set_bg_color(screen, 0xF6F8FAU);

    title = tinyui_label_create(screen);
    qrcode = tinyui_qrcode_create(screen);
    if (title == NULL || qrcode == NULL) {
        return TINYUI_ERROR_NO_MEMORY;
    }

    if (tinyui_obj_set_pos(title, 32, 24) != TINYUI_OK
        || tinyui_obj_set_size(title, 200, 28) != TINYUI_OK
        || tinyui_label_set_text(title, "QR Code") != 0
        || tinyui_label_set_text_color(title, 0x102030U) != 0) {
        return TINYUI_ERROR_BACKEND;
    }

    if (tinyui_obj_set_pos(qrcode, 176, 80) != TINYUI_OK
        || tinyui_obj_set_size(qrcode, 128, 128) != TINYUI_OK
        || tinyui_qrcode_set_text(qrcode, "https://example.local/tinyui") != 0) {
        return TINYUI_ERROR_BACKEND;
    }

    printf("TINYUI_SCENARIO=qrcode_basic\n");
    fflush(stdout);
    return TINYUI_OK;
}
