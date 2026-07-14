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

#ifndef TINYUI_QRCODE_H
#define TINYUI_QRCODE_H

#include "core/obj.h"
#include <stdint.h>

typedef enum tinyui_qrcode_field {
    TINYUI_QRCODE_FIELD_ID = UINT32_C(1) << 0,
    TINYUI_QRCODE_FIELD_STYLE_CLASS = UINT32_C(1) << 1,
    TINYUI_QRCODE_FIELD_USER_DATA = UINT32_C(1) << 2,
    TINYUI_QRCODE_FIELD_TEXT = UINT32_C(1) << 3,
    TINYUI_QRCODE_FIELD_QR_COLOR = UINT32_C(1) << 4,
    TINYUI_QRCODE_FIELD_BG_COLOR = UINT32_C(1) << 5,
    TINYUI_QRCODE_FIELD_ECC = UINT32_C(1) << 6,
    TINYUI_QRCODE_FIELD_MAX_VERSION = UINT32_C(1) << 7,
    TINYUI_QRCODE_FIELD_ZOOM = UINT32_C(1) << 8,
} tinyui_qrcode_field_t;

typedef struct tinyui_qrcode_props {
    uint32_t fields;
    uint16_t id;
    const char *style_class;
    void *user_data;
    const char *text;
    unsigned int qr_color;
    unsigned int bg_color;
    int ecc;
    int max_version;
    int zoom;
} tinyui_qrcode_props_t;

tinyui_obj_t *tinyui_qrcode_create(tinyui_obj_t *parent);

tinyui_obj_t *tinyui_qrcode_create_with_props(tinyui_obj_t *parent,
                                               const tinyui_qrcode_props_t *props);

int tinyui_qrcode_set_text(tinyui_obj_t *qrcode, const char *text);

const char *tinyui_qrcode_get_text(const tinyui_obj_t *qrcode);


int tinyui_qrcode_set_qr_color(tinyui_obj_t *qrcode, unsigned int rgb);

int tinyui_qrcode_set_bg_color(tinyui_obj_t *qrcode, unsigned int rgb);

int tinyui_qrcode_set_ecc(tinyui_obj_t *qrcode, int ecc);

int tinyui_qrcode_set_max_version(tinyui_obj_t *qrcode, int max_version);

int tinyui_qrcode_set_zoom(tinyui_obj_t *qrcode, int zoom);

#endif /* TINYUI_QRCODE_H */
