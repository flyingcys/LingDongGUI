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

struct tinyui_widget;
struct tinyui_qrcode;

enum tinyui_qrcode_prop_mask {
    TINYUI_QRCODE_PROP_QR_COLOR = 1 << 0,
    TINYUI_QRCODE_PROP_BG_COLOR = 1 << 1,
    TINYUI_QRCODE_PROP_ECC = 1 << 2,
    TINYUI_QRCODE_PROP_MAX_VERSION = 1 << 3,
    TINYUI_QRCODE_PROP_ZOOM = 1 << 4,
};

struct tinyui_qrcode_props {
    const char *id;
    const char *style_class;
    void *user_data;
    const char *text;
    unsigned int qr_color;
    unsigned int bg_color;
    int ecc;
    int max_version;
    int zoom;
    unsigned int present_mask;
};

struct tinyui_qrcode *tinyui_qrcode_create(struct tinyui_widget *parent, const char *id);
struct tinyui_qrcode *tinyui_qrcode_create_with_props(struct tinyui_widget *parent,
                                                      const struct tinyui_qrcode_props *props);
struct tinyui_qrcode *tinyui_q_r_code_init(struct tinyui_widget *parent, const char *id);
int tinyui_qrcode_set_text(struct tinyui_qrcode *qrcode, const char *text);
const char *tinyui_qrcode_get_text(const struct tinyui_qrcode *qrcode);
int tinyui_q_r_code_set_text(struct tinyui_qrcode *qrcode, const char *text);
int tinyui_qrcode_set_qr_color(struct tinyui_qrcode *qrcode, unsigned int rgb);
int tinyui_qrcode_set_bg_color(struct tinyui_qrcode *qrcode, unsigned int rgb);
int tinyui_qrcode_set_ecc(struct tinyui_qrcode *qrcode, int ecc);
int tinyui_qrcode_set_max_version(struct tinyui_qrcode *qrcode, int max_version);
int tinyui_qrcode_set_zoom(struct tinyui_qrcode *qrcode, int zoom);

#endif
